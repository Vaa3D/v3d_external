#ifndef HTTPUTILSQUALITYINSPECTION_H
#define HTTPUTILSQUALITYINSPECTION_H

#include <QObject>
#include "net/httputils.h"
#include <QApplication>
#include <QFile>
#include <QJsonArray>

class HttpUtilsQualityInspection: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsQualityInspection(QObject *parent = nullptr);
    ~HttpUtilsQualityInspection();


    void getarborWithHttp(int Maxid=0);
    void updateArborResult(int arborid, int result);
    virtual void asyncPostRequest(QString url, QJsonObject &body);
//    void bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);
public slots:

    void arborReply(QNetworkReply* reply);
    void updateReply(QNetworkReply* reply);
signals:
    void sendarborinfo(int arborid,QString name,QString somaId,QString image,double x,double y,double z);
    void readytodownload();
private:
    QString URL_GET_ARBOR = SERVER_IP + "/dynamic/arbor/getarbor";
    QString URL_QUERY_ARBOR_RESULT = SERVER_IP + "/dynamic/arbor/queryarborresult";
    QString URL_UPDATE_ARBOR_RESULT = SERVER_IP + "/dynamic/arbor/updatearborresult";
    QString URL_GET_SWC = SERVER_IP + "/dynamic/swc/cropswc";
    QString URL_GET_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbor/queryarborresult";
    QString URL_QUERY_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/query";
    QString URL_INSERT_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/insert";
    QString URL_DELETE_ARBOR_MARKER_LIST = SERVER_IP + "/dynamic/arbordetail/delete";

    QNetworkAccessManager *managerarbor;
    QNetworkAccessManager *managerupdate;
    QString mbrainId;
    QString mres;
    int moffsetX, moffsetY, moffsetZ;
};

#endif // HTTPUTILSQUALITYINSPECTION_H
