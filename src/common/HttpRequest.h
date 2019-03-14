#pragma once

#include <QObject>
#include <QNetworkReply>
#include <functional>

class QNetworkAccessManager;
class HttpRequest : public QObject
{
    Q_OBJECT

public:
    HttpRequest(QObject *parent = nullptr);
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

class HttpRequestCenter : public QObject
{
public:
    typedef std::function<void(int err, const QString &url, const QByteArray &data)> ResponseCallback;

    static HttpRequestCenter* instance()
    {
        static HttpRequestCenter s_inst;
        return &s_inst;
    }

    void get(const QString &url, const ResponseCallback &cb)
    {
        HttpRequest *request = new HttpRequest();
        connect(request, &HttpRequest::sigHttpResponse, [=](int err, const QByteArray &data) {
            cb(err, url, data);
            request->deleteLater();
        });
        request->get(url);
    }
};
