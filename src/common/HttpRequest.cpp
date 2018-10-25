#include "HttpRequest.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSsl>

HttpRequest::HttpRequest(QObject *parent)
    : QObject(parent)
    , manager_(new QNetworkAccessManager(this))
{
    connect(manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)), Qt::QueuedConnection);
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::get(const QString &url, const QString &type)
{
    type_ = type;
    url_ = url;
    QNetworkRequest request;
    QSslConfiguration config;
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1_0);
    request.setSslConfiguration(config);
    request.setUrl(url);

    QNetworkReply *reply = manager_->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void HttpRequest::post(const QString &url, const QByteArray &data)
{
    qDebug() << "url:" << url << "," << data;
    url_ = url;
    QUrl aurl(url);
    QNetworkRequest req(aurl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = manager_->post(req, data);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void HttpRequest::replyFinished(QNetworkReply *reply)
{
    if (!reply) {
        qDebug() << "replyFinished is null";
        return;
    }

    emit sigHttpResponse(0, reply->readAll());
    reply->deleteLater();
}

void HttpRequest::slotError(QNetworkReply::NetworkError err)
{
    qDebug() << "http request reply error:" << err;
}
