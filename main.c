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


int main(int argc, char *argv[]) {
  SSL_load_error_strings();
  SSL_library_init();
  SSL_CTX * sslContext = SSL_CTX_new(SSLv23_client_method());
  if (sslContext == NULL) { ERR_print_errors_fp(stderr); return 1; }

  static char fileName[128] = {0};
  time_t t = time(NULL);
  strftime(fileName, sizeof(fileName), "Screenshots/Screenshot-alkedr_%Y.%m.%d_%H.%M.%S.png", localtime(&t));

  SSL * handle = sslConnect(sslContext, "webdav.yandex.com", 443);
  if (handle == NULL) { return 1; }

  void * scr = malloc(1024*1024);
  int screenshotSize;
  takeScreenshot(scr, &screenshotSize);

  static char request[1024*1024];

  SSL_write(handle, request, sprintf(request,
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
  ));

  SSL_write(handle, request, sprintf(request,
    "PROPPATCH /%s HTTP/1.1\r\n"
    "Host: webdav.yandex.ru\r\n"
    "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    "Content-Length: 128\r\n"
    "\r\n"
    "<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>"
    "\r\n",
    fileName
  ));

  SSL_write(handle, request, sprintf(request,
    "PUT /%s HTTP/1.1\r\n"
    "Host: webdav.yandex.ru\r\n"
    "Accept: */*\r\n"
    "Authorization: OAuth " OAUTH_TOKEN "\r\n"
    "Expect: 100-continue\r\n"
    "Content-Type: application/binary\r\n"
    "Content-Length: %d\r\n"
    "\r\n",
    fileName,
    screenshotSize
  ));
  SSL_write(handle, scr, screenshotSize);

  static char response[10240];

  SSL_read(handle, response, sizeof(response));
  SSL_read(handle, response, sizeof(response));
  SSL_read(handle, response, sizeof(response));
  memset(response, 0, sizeof(response));
  SSL_read(handle, response, sizeof(response));

  const char * linkBegin = strstr(response, "https://yadi.sk/");
  char * linkEnd = strstr(response, "</public_url>");
  *linkEnd = 0;
  printf("%s", linkBegin);
  fflush(stdin);

  SSL_read(handle, response, sizeof(response));
  SSL_read(handle, response, sizeof(response));

  SSL_shutdown(handle);
  close(SSL_get_fd(handle));

  return 0;
}
