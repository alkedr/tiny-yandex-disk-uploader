#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


static void log(const char * s) {
  static long long startTime = 0;

  struct timeval currentTimeval;
  gettimeofday(&currentTimeval, NULL);
  long long currentTime = currentTimeval.tv_sec * 1000 + currentTimeval.tv_usec / 1000;
  if (startTime) {
    printf("%lldms\n", currentTime - startTime);
  }
  printf("%s  ", s);
  fflush(stdout);
  startTime = currentTime;
}


struct Screenshot {
  char * data;
  int size;
};

static cairo_status_t writeScreenshotDataCairoCallback(void * userData, const unsigned char *data, unsigned int length) {
  struct Screenshot * screenshot = (struct Screenshot*)userData;
  memcpy(screenshot->data + screenshot->size, data, length);
  screenshot->size += length;
  return CAIRO_STATUS_SUCCESS;
}

static void takeScreenshot(char * data, int * size) {
  Display * disp = XOpenDisplay(NULL);
  cairo_surface_t * surface = cairo_xlib_surface_create(
    disp, 
    DefaultRootWindow(disp), 
    DefaultVisual(disp, DefaultScreen(disp)), 
    DisplayWidth(disp, DefaultScreen(disp)), 
    DisplayHeight(disp, DefaultScreen(disp))
  );
  struct Screenshot screenshot;
  screenshot.data = data;
  screenshot.size = 0;
  cairo_surface_write_to_png_stream(surface, writeScreenshotDataCairoCallback, &screenshot);
  cairo_surface_destroy(surface);
  *size = screenshot.size;
}


static SSL * sslConnect(SSL_CTX * sslContext, const char * hostName, uint16_t port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) { perror("socket"); return NULL; }

  struct hostent * host = gethostbyname(hostName);   // TODO getaddrinfo
  if (host == NULL) { perror("gethostbyname"); return NULL; }

  struct sockaddr_in server = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
    .sin_addr = *((struct in_addr*)host->h_addr_list[0]),
  };
  int error = connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr));
  if (error == -1) { perror("connect"); return 0; }

  SSL * sslHandle = SSL_new(sslContext);
  if (sslHandle == NULL) { ERR_print_errors_fp(stderr); }

  if (!SSL_set_fd(sslHandle, sock)) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }

  if (SSL_connect(sslHandle) != 1) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }
  return sslHandle;
}



#define OAUTH_TOKEN "45e24fd66c884bafaae7cc4e2e462789"



const void writePutEmptyFileRequest(SSL * handle, const char * fileName) {
  static char request[1024*1024];
  int requestLength = sprintf(request,
    "PUT /%s HTTP/1.1\r\n"
    "Host: webdav.yandex.ru\r\n"
    "Accept: */*\r\n"
    "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    "Etag: d41d8cd98f00b204e9800998ecf8427e\r\n"
    "Sha256: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\r\n"
    "Expect: 201-created\r\n"
    "Content-Type: application/binary\r\n"
    "Content-Length: 0\r\n"
    "\r\n",
    fileName
  );
  SSL_write(handle, request, requestLength);
}

const void writePublishFileRequest(SSL * handle, const char * fileName) {
  static char buffer[1024*1024];
  int requestLength = sprintf(buffer,
    "PROPPATCH /%s HTTP/1.1\r\n"
    "Host: webdav.yandex.ru\r\n"
    "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    "Content-Length: 128\r\n"
    "\r\n"
    "<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>"
    "\r\n",
    fileName
  );
  SSL_write(handle, buffer, requestLength);
}

const void writePutFileRequest(SSL * handle, const char * fileName, void * contents, int size) {
  static char buffer[1024*1024];
  int requestLength = sprintf(buffer,
    "PUT /%s HTTP/1.1\r\n"
    "Host: webdav.yandex.ru\r\n"
    "Accept: */*\r\n"
    "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    "Expect: 100-continue\r\n"
    "Content-Type: application/binary\r\n"
    "Content-Length: %d\r\n"
    "\r\n",
    fileName,
    size
  );
  SSL_write(handle, buffer, requestLength);
  SSL_write(handle, contents, size);
}





int main() {
  const char * fileName = "8.png";

  SSL_load_error_strings();
  SSL_library_init();
  SSL_CTX * sslContext = SSL_CTX_new(SSLv23_client_method());
  if (sslContext == NULL) { ERR_print_errors_fp(stderr); return 1; }

  log("Connecting");
  SSL * handle = sslConnect(sslContext, "webdav.yandex.com", 443);
  if (handle == NULL) { return 1; }


  log("Taking screenshot");
  void * scr = malloc(1024*1024);
  int screenshotSize;
  takeScreenshot(scr, &screenshotSize);

  log("Sending requests");
  writePutEmptyFileRequest(handle, fileName);

  writePublishFileRequest(handle, fileName);

  writePutFileRequest(handle, fileName, scr, screenshotSize);

  log("Receiving response");
  static char response[10240];

  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));

  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));

  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));

  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));

  log("Receiving response 2");
  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));

  SSL_read(handle, response, sizeof(response));
  printf("\n\n%s", response);
  memset(response, 0, sizeof(response));



  // static char request[1024*1024];
  // int requestLength = 0;
  // requestLength += sprintf(request + requestLength,
    // "PUT /%s HTTP/1.1\r\n"
    // "Host: webdav.yandex.ru\r\n"
    // "Accept: */*\r\n"
    // "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    // "Content-Type: image/png\r\n"
    // "Content-Length: 0\r\n"
    // "Etag: d41d8cd98f00b204e9800998ecf8427e\r\n"
    // "Sha256: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\r\n"
    // "\r\n"
    // "PROPPATCH /%s HTTP/1.1\r\n"
    // "Host: webdav.yandex.ru\r\n"
    // "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    // "Content-Length: 128\r\n"
    // "\r\n"
    // "<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>"
    // "\r\n"
    // "\r\n"
    // "\r\n"
    // "PUT /%s HTTP/1.1\r\n"
    // "Host: webdav.yandex.ru\r\n"
    // "Accept: */*\r\n"
    // "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    // "Expect: 100-continue\r\n"
    // "Content-Type: image/png\r\n"
    // "Content-Length: ",

// "PUT /%s HTTP/1.1\r\n"
// "Host: webdav.yandex.ru\r\n"
// "Accept: */*\r\n"
// "Authorization: OAuth " OAUTH_TOKEN "\r\n"
// "Expect: 100-continue\r\n"
// "Content-Type: application/binary\r\n"
// "Content-Length: ",
//     // fileName,
//     // fileName,
//     fileName
//   );
//   int screenshotSizeOffset = requestLength;
//   requestLength += sprintf(request + requestLength, "          \r\n\r\n");
//   int screenshotOffset = requestLength;
//   int screenshotSize = 0;
//   takeScreenshot(request + requestLength, &screenshotSize);
//   int screenshotSizeLength = sprintf(request + screenshotSizeOffset, "%d", screenshotSize);
//   request[screenshotSizeOffset + screenshotSizeLength] = ' ';

//   printf("%s\n", request);




  // log("Writing request");
  // SSL_write(handle, request, requestLength-screenshotSize);
  // SSL_write(handle, request+screenshotOffset, screenshotSize);

  // log("Reading response");
  // static char response[10240];
  // // while (SSL_read(handle, response, sizeof(response))) {}
  // SSL_read(handle, response, sizeof(response));
  // printf("\n\n%s", response);
  // memset(response, 0, sizeof(response));

  // SSL_read(handle, response, sizeof(response));
  // printf(response);
  // memset(response, 0, sizeof(response));

  // SSL_read(handle, response, sizeof(response));
  // printf(response);
  // memset(response, 0, sizeof(response));

  // SSL_read(handle, response, sizeof(response));
  // printf(response);
  // memset(response, 0, sizeof(response));

  // SSL_read(handle, response, sizeof(response));
  // printf(response);
  // memset(response, 0, sizeof(response));

  // SSL_read(handle, response, sizeof(response));
  // printf(response);
  // memset(response, 0, sizeof(response));




  //   "         \r\n"
  //   "\r\n",

  //   fileName,
  //   fileName,
  //   strlen(BODY), 
  //   BODY,
  //   "4.png",
  //   screenshot.size
  // );












  // log("Taking screenshot");
  // struct Screenshot screenshot = takeScreenshot();
  // screenshot.data[screenshot.size++] = '\r';
  // screenshot.data[screenshot.size++] = '\n';

  // log("Sending request");
  // static char request[1024];
  // int requestLength = sprintf(request,
  //   "PUT /%s HTTP/1.1\r\n"
  //   "Host: webdav.yandex.ru\r\n"
  //   "Accept: */*\r\n"
  //   "Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
  //   "Expect: 100-continue\r\n"
  //   "Content-Type: image/png\r\n"
  //   "Content-Length: 0\r\n"
  //   "\r\n"
  //   "PROPPATCH /%s HTTP/1.1\r\n"
  //   "Host: webdav.yandex.ru\r\n"
  //   "Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
  //   "Content-Length: %lu\r\n"
  //   "\r\n"
  //   "%s"
  //   "\r\n"
  //   "PUT /%s HTTP/1.1\r\n"
  //   "Host: webdav.yandex.ru\r\n"
  //   "Accept: */*\r\n"
  //   "Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
  //   "Expect: 100-continue\r\n"
  //   "Content-Type: image/png\r\n"
  //   "Content-Length: %d\r\n"
  //   "\r\n",

  //   "4.png",
  //   "4.png",
  //   strlen(BODY), 
  //   BODY,
  //   "4.png",
  //   screenshot.size
  // );
  // SSL_write(handle, request, requestLength);
  // SSL_write(handle, screenshot.data, screenshot.size);

  // static char response[10240];
  // // while (SSL_read(handle, response, sizeof(response))) {}
  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);

  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);

  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);

  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);

  // SSL_read(handle, response, sizeof(response));
  // printf("\n%s\n\n", response);



  // log("Replacing empty file with screenshot");
  // requestLength = sprintf(request,
  // );
  // SSL_write(handle, request, requestLength);

  // SSL_read(handle, response, sizeof(response));


  // SSL_read(handle, response, sizeof(response));

  // requestLength = sprintf(request, ,
  //   "4.png", strlen(BODY), BODY
  // );



  // log("Sending PUT request");
  // static char request[1024];
  // int requestLength = sprintf(request,
  //   "PUT /%s HTTP/1.1\r\n"
  //   "Host: webdav.yandex.ru\r\n"
  //   "Accept: */*\r\n"
  //   "Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
  //   "Expect: 100-continue\r\n"
  //   "Content-Type: image/png\r\n"
  //   "Content-Length: %d\r\n"
  //   "\r\n",
  //   "4.png",
  //   screenshot.size
  // );
  // SSL_write(handle, request, requestLength);

  // // log("Receiving response");
  // static char response[10240];
  // // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);

  // // TODO: check that it is 100 CONTINUE
  // // printf("|||%u|||\n", strlen("<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>"));

  // log("Sending screenshot");
  // screenshot.data[screenshot.size++] = '\r';
  // screenshot.data[screenshot.size++] = '\n';
  // SSL_write(handle, screenshot.data, screenshot.size);

  // // log("Receiving response");
  // // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);


  // log("Sending publish request");
  // static const char BODY[] = "<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>";
  // requestLength = sprintf(request, 
  //   "PROPPATCH /%s HTTP/1.1\r\n"
  //   "Host: webdav.yandex.ru\r\n"
  //   "Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
  //   "Content-Length: %lu\r\n"
  //   "\r\n"
  //   "%s",
  //   "4.png", strlen(BODY), BODY
  // );
  // // puts(request);
  // SSL_write(handle, request, requestLength);

  // log("Receiving response");
  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);
  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);
  // SSL_read(handle, response, sizeof(response));
  // // printf("\n%s\n\n", response);
  // SSL_read(handle, response, sizeof(response));
  // printf("\n%s\n\n", response);

  SSL_shutdown(handle);
  close(SSL_get_fd(handle));

  log("Done\n");

  return 0;
}
