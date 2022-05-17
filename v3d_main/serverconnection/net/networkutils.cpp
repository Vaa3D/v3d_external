#include "networkutils.h"

#include <QJsonDocument>

class NetWorkUtil::Private {
public:
    Private(NetWorkUtil *q) :
        manager(new QNetworkAccessManager(q))
    { }
    QNetworkAccessManager* manager;
};


NetWorkUtil *NetWorkUtil::instance()
{
    static NetWorkUtil networkUtil;
    return &networkUtil;
}

QNetworkReply *NetWorkUtil::postRequst(const QString &url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);
    return d->manager->post(request, dataArray);
}

NetWorkUtil::~NetWorkUtil()
{
    delete d;
    d = nullptr;
}

NetWorkUtil::NetWorkUtil(QObject *parent) : QObject(parent)
{
    d = new NetWorkUtil::Private(this);
    connect(d->manager, &QNetworkAccessManager::finished, this, &NetWorkUtil::finished);

    Q_OVERRIDE()
}
