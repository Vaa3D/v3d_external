#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class HttpUtils: public QObject
{
    Q_OBJECT
public:
    HttpUtils(QObject *parent = nullptr);
    virtual ~HttpUtils();

    virtual void asyncPostRequest(QString url, QJsonObject &body);

//public slots:
//    virtual void replyFinished(QNetworkReply* reply);

protected:
    const QString SERVER_IP  = "http://139.155.28.154:26000";
    QNetworkAccessManager *manager = nullptr;
    QNetworkRequest request;
};

#endif // HTTPUTILS_H
