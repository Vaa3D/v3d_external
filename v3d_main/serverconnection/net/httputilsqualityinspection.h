#ifndef HTTPUTILSQUALITYINSPECTION_H
#define HTTPUTILSQUALITYINSPECTION_H

#include <QObject>
#include "net/httputils.h"

class HttpUtilsQualityInspection: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsQualityInspection(QWidget *parent = nullptr);
    ~HttpUtilsQualityInspection();

    void getSWCWithHttp(QString res, float offsetX, float offsetY, float offsetZ, int size, QString arborName);
    virtual void asyncPostRequest(QString url, QJsonObject &body);

public slots:
    void SWCReply(QNetworkReply* reply);

private:
    QString URL_GET_ARBOR = SERVER_IP + "/dynamic/arbor/getarbor";
    QString URL_QUERY_ARBOR_RESULT = SERVER_IP + "/dynamic/arbor/queryarborresult";
    QString URL_UPDATE_ARBOR_RESULT = SERVER_IP + "/dynamic/arbor/updatearborresult";
    QString URL_GET_SWC = SERVER_IP + "/dynamic/swc/cropswc";
    QString URL_GET_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbor/queryarborresult";
    QString URL_QUERY_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/query";
    QString URL_INSERT_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/insert";
    QString URL_DELETE_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/delete";
};

#endif // HTTPUTILSQUALITYINSPECTION_H
