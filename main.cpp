#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <iostream>
#include <boost/thread/future.hpp>


static QByteArray makeScreenshot() {
	QByteArray screenshotByteArray;
	QBuffer buffer(&screenshotByteArray);
	buffer.open(QIODevice::WriteOnly);
	QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId()).save(&buffer, "PNG");
	return screenshotByteArray;
}

static QNetworkRequest request(QString url) {
	auto result = QNetworkRequest{QUrl(url)};
	result.setRawHeader("Authorization", "OAuth 5f8dd1f375f945d7945247f8c4820009");
	return result;
}

static QNetworkRequest pathRequest(QString fileName) {
	return request("https://cloud-api.yandex.net/v1/disk/resources/upload?path=" + fileName + "&overwrite=true");
}

static QNetworkRequest publishRequest(QString fileName) {
	return request("https://cloud-api.yandex.net/v1/disk/resources/upload?path=" + fileName + "&overwrite=true");
}



QNetworkAccessManager qnam;


// static void retrieveUploadUrl


static void uploadFile(QString name, QString url, QByteArray contents) {
	QNetworkReply * uploadResponse = qnam.put(request(url), screenshotByteArray);
}


static void retrieveUploadUrl(QString name, QByteArray contents) {
	QNetworkReply * pathResponse = qnam.get(pathRequest("123.png"));
	QObject::connect(pathResponse, &QNetworkReply::finished, 
		[&]() {
			auto json = QJsonDocument::fromJson(pathResponse->readAll()).object();
			std::cout << json.toJson() << std::endl;
			std::cout << "Урл для загрузки получен: " << json["href"].toString() << std::endl;
			uploadFile(name, json["href"].toString(), contents);
		}
	);
}


int main(int argc, char ** argv) {
	QApplication app(argc, argv);
	QApplication::setFont(QFont("Arial", 20));

	QByteArray screenshotByteArray = makeScreenshot();

	// auto retrieveUploadUrl = [](QString fileName) {

	// }




	QNetworkReply * pathResponse = qnam.get(pathRequest("123.png"));
	QObject::connect(pathResponse, &QNetworkReply::finished, 
		[&]() {
			// std::cout << QString(pathResponse->readAll()).toStdString() << std::endl;
			auto json = QJsonDocument::fromJson(pathResponse->readAll()).object();
			QNetworkReply * uploadResponse = qnam.put(request(json["href"].toString()), screenshotByteArray);
			QObject::connect(uploadResponse, &QNetworkReply::finished,
				[&]() {
					std::cout << "Файл загружен" << std::endl;
					auto body = new QByteArray("<propertyupdate xmlns=\"DAV:\"><set><prop><public_url xmlns=\"urn:yandex:disk:meta\">true</public_url></prop></set></propertyupdate>");
					auto buffer = new QBuffer(body);
					buffer->open(QIODevice::ReadOnly);
					QNetworkReply * publishResponse = qnam.sendCustomRequest(
						publishRequest("123.png"), 
						"PROPPATCH",
						buffer
					);
					QObject::connect(publishResponse, &QNetworkReply::finished, 
						[&]() {
							std::cout << "Файл расшарен" << std::endl;
							std::cout << QString(pathResponse->readAll()).toStdString() << std::endl;
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
