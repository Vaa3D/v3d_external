#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QObject>
#include <QNetworkReply>
#include <QMap>

class NetWorkUtil : public QObject
{
    Q_OBJECT
public:
    // public functions
    static NetWorkUtil *instance();

    QNetworkReply * postRequst(const QString &url, QJsonObject &body);
    ~NetWorkUtil();

public:
    // public member
    enum RequestType {
        LOGIN,
        POTENTIALLOCATION,
        BRAINLIST,
        DOWNLOADIMAGE
    };
    QMap<QNetworkReply*, RequestType> typeMap; // 每次进行数据获取，将返回对象的reply*与枚举类型做对应，以区分操作

signals:
    void finished(QNetworkReply * reply);

public slots:


private:
    explicit NetWorkUtil(QObject *parent = nullptr);
    NetWorkUtil(NetWorkUtil &) = delete;
    NetWorkUtil &operator=(NetWorkUtil nwu) = delete;

private:
    class Private;
    friend class Private;
    Private * d;
    QNetworkRequest request;
};

#endif // NETWORKUTILS_H
