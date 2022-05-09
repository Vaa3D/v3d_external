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
    HttpUtils(QWidget *parent = nullptr);

    virtual void asyncPostRequest(QUrl url, QJsonObject body);

signals:
//    void loginSuccess();

public slots:
    virtual void replyFinished(QNetworkReply* reply);

protected:
    const QString SERVER_IP  = "http://139.155.28.154:26000";
    QNetworkAccessManager *manager;
    QNetworkRequest request;
};

#endif // HTTPUTILS_H
