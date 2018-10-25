#pragma once

#include <QObject>
#include <QNetworkReply>

class QNetworkAccessManager;
class HttpRequest : public QObject
{
    Q_OBJECT

public:
    HttpRequest(QObject *parent);
    ~HttpRequest();
    void get(const QString &url, const QString &type = "");
    void post(const QString &url, const QByteArray &data);
    const QString& type() { return type_; }
    const QString& url() { return url_; }


signals:
    void sigHttpResponse(int err, const QByteArray &data);

private slots:
    void replyFinished(QNetworkReply*);
    void slotError(QNetworkReply::NetworkError);

private:
    QNetworkAccessManager *manager_;
    QString type_;
    QString url_;
};
