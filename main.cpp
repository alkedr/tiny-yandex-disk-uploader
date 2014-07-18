#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <iostream>


static QString sendRequest(QByteArray request) {
	QSslSocket socket;
	socket.connectToHostEncrypted("webdav.yandex.com", 443);
	socket.write(request.data(), request.size());
	socket.flush();
	QString response;
	while (socket.waitForReadyRead()) {
		std::cout << "read" << std::endl;
		response += socket.readAll().data();
	}
	return response;
}


int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);

	auto s2 =
			"PROPFIND / HTTP/1.1\r\n"
			"Host: webdav.yandex.ru\r\n"
			"Accept: */*\r\n"
			"Depth: 1\r\n"
			"Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n\r\n"
		;


	auto putResponse = sendRequest(
		QByteArray(
			QString(
				"PUT /%1 HTTP/1.1\r\n"
				"Host: webdav.yandex.ru\r\n"
				"Accept: */*\r\n"
				"Authorization: OAuth 45e24fd66c884bafaae7cc4e2e462789\r\n"
				"Expect: 100-continue\r\n"
				"Content-Type: image/png\r\n"
				"Content-Length: %2\r\n"
				"\r\n"
			)
			.arg("1.png")
			.arg(123)
			.toStdString().data()
		)
	);
	qDebug() << putResponse;
}

















/*
// #include <boost/thread/future.hpp>
















static QByteArray makeScreenshot() {
	QByteArray screenshotByteArray;
	QBuffer buffer(&screenshotByteArray);
	buffer.open(QIODevice::WriteOnly);
	QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId()).save(&buffer, "PNG");
	return screenshotByteArray;
}

static QNetworkRequest request(QString url) {
	auto result = QNetworkRequest{QUrl(url)};
	result.setRawHeader("Authorization", "OAuth 45e24fd66c884bafaae7cc4e2e462789");
	return result;
}

static QNetworkRequest pathRequest(QString fileName) {
	return request("https://cloud-api.yandex.net/v1/disk/resources/upload?path=" + fileName + "&overwrite=true");
}

static QNetworkRequest publishRequest(QString fileName) {
	return request(fileName);
}


template<class Callback> void retrieveUploadUrl(QNetworkAccessManager * qnam, QString fileName, Callback callback) {
	QNetworkReply * pathResponse = qnam->get(pathRequest(fileName));
<<<<<<< HEAD
	QObject::connect(pathResponse, &QNetworkReply::finished,
=======
	QObject::connect(pathResponse, &QNetworkReply::finished,
>>>>>>> 3fe7f7cf0c7f879271d92a715886c6a012327382
		[&]() {
			auto json = QJsonDocument::fromJson(pathResponse->readAll()).object();
			// std::cout << json.toJson() << std::endl;
			std::cout << "Урл для загрузки получен: " << json["href"].toString().toStdString() << std::endl;
			callback(json["href"].toString());
		}
	);
}

template<class Callback> void upload(QNetworkAccessManager * qnam, QString url, QByteArray contents, Callback callback) {
	QNetworkReply * uploadResponse = qnam->put(request(url), contents);
	QObject::connect(uploadResponse, &QNetworkReply::finished,
		[&]() {
			std::cout << "Файл загружен" << std::endl;
			callback();
		}
	);
}

template<class Callback> void publish(QNetworkAccessManager * qnam, QString fileName, Callback callback) {
	auto body = new QByteArray("<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>");
	auto buffer = new QBuffer(body);
	buffer->open(QIODevice::ReadOnly);
	QNetworkReply * publishResponse = qnam->sendCustomRequest(publishRequest(fileName), "PROPPATCH", buffer);
<<<<<<< HEAD
	QObject::connect(publishResponse, &QNetworkReply::finished,
=======
	QObject::connect(publishResponse, &QNetworkReply::finished,
>>>>>>> 3fe7f7cf0c7f879271d92a715886c6a012327382
		[&]() {
			std::cout << "Файл расшарен" << std::endl;
			std::cout << QString(publishResponse->readAll()).toStdString() << std::endl;
			callback("");
		}
	);
}





// static void retrieveUploadUrl


// static void uploadFile(QString name, QString url, QByteArray contents) {
// 	QNetworkReply * uploadResponse = qnam.put(request(url), screenshotByteArray);
// }


// static void retrieveUploadUrl(QString name, QByteArray contents) {
// 	QNetworkReply * pathResponse = qnam.get(pathRequest("123.png"));
<<<<<<< HEAD
// 	QObject::connect(pathResponse, &QNetworkReply::finished,
=======
// 	QObject::connect(pathResponse, &QNetworkReply::finished,
>>>>>>> 3fe7f7cf0c7f879271d92a715886c6a012327382
// 		[&]() {
// 			auto json = QJsonDocument::fromJson(pathResponse->readAll()).object();
// 			std::cout << json.toJson() << std::endl;
// 			std::cout << "Урл для загрузки получен: " << json["href"].toString() << std::endl;
// 			uploadFile(name, json["href"].toString(), contents);
// 		}
// 	);
// }


int main(int argc, char ** argv) {
	QApplication app(argc, argv);
	QApplication::setFont(QFont("Arial", 20));

	QByteArray screenshotByteArray = makeScreenshot();

	// auto retrieveUploadUrl = [](QString fileName) {

	// }


QNetworkAccessManager qnam;


	std::cout << "Получаем урл для загрузки" << std::endl;
	QNetworkReply * pathResponse = qnam.get(pathRequest("123.png"));
	QObject::connect(pathResponse, &QNetworkReply::finished,
		[&]() {
			auto json = QJsonDocument::fromJson(pathResponse->readAll()).object();
			std::cout << "Урл для загрузки получен: " << json["href"].toString().toStdString() << std::endl;
			// std::cout << QString(pathResponse->readAll()).toStdString() << std::endl;
			std::cout << "Загружаем файл" << std::endl;
			QNetworkReply * uploadResponse = qnam.put(request(json["href"].toString()), screenshotByteArray);
			QObject::connect(uploadResponse, &QNetworkReply::finished,
				[&]() {
					std::cout << "Файл загружен" << std::endl;
					auto body = new QByteArray("<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>");
					auto buffer = new QBuffer(body);
					buffer->open(QIODevice::ReadOnly);
					std::cout << "Публикуем файл" << std::endl;

					QNetworkRequest req = request("https://webdav.yandex.ru");

// User-Agent: my_application/0.0.1
// Host: webdav.yandex.ru
// Authorization: OAuth 0c4182a7c2cf4521964a72ff57a34a07
// Content-Length: 158


					req.setRawHeader("Host", "webdav.yandex.ru");
					req.setRawHeader("Accept", "**");
					req.setRawHeader("Expect", "100-continue");
					req.setRawHeader("Content-Length", QByteArray(QString::number(body->size()).toStdString().c_str()));

					QNetworkReply * publishResponse = qnam.sendCustomRequest(
<<<<<<< HEAD
						req,
=======
						req,
>>>>>>> 3fe7f7cf0c7f879271d92a715886c6a012327382
						"PROPPATCH /123.png HTTP/1.1",
						buffer
					);
					QObject::connect(publishResponse, &QNetworkReply::finished,
						[&]() {
							std::cout << "Файл опубликован" << std::endl;

							std::cout << publishResponse << std::endl;
							std::cout << publishResponse->rawHeaderPairs().size() << std::endl;

							QList<QByteArray> headerList = publishResponse->rawHeaderList();
							for (auto head : headerList) {
							    qDebug() << head << ":" << publishResponse->rawHeader(head);
							}

							// for (auto pair : publishResponse->rawHeaderPairs()) {
							//     qDebug() << pair.first << ":" << pair.second;
							// }

							// QList<QByteArray> headerList = ;
							// foreach(QByteArray head, reply->rawHeaderList()) {
							//     qDebug() << head << ":" << reply->rawHeader(head);
							// }


							std::cout << QString(pathResponse->readAll()).toStdString() << std::endl;
							std::cout << pathResponse->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString().toStdString() << std::endl;
							std::cout << pathResponse->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().toStdString() << std::endl;
							// QApplication::clipboard()->setText();
							// QClipboard *clipboard = QApplication::clipboard();
							std::cout << "finished" << std::endl;
						}
					);
				}
			);
		}
	);

	return app.exec();
}


*/
