#ifndef HTTPUTILSIMAGE_H
#define HTTPUTILSIMAGE_H

#include <QJsonObject>
#include <QString>

#include "serverconnection/net/httputils.h"

class HttpUtilsImage: public HttpUtils
{
public:
    HttpUtilsImage();

//    void getBrainListWithHttp();
    void downloadImageWithHttp(QJsonObject userInfo, QJsonObject bBox);
    void getBBSwcWithHttp(QString username, QString password, QString swc,
                          int res, int x, int y, int z, int len);
    void downloadSomaBlockWithHttp(QString username, QString password, QString swc,
                                   int res, int x, int y, int z, int len);

private:
    const QString URL_GET_BRAIN_LIST = SERVER_IP + "/dynamic/image/getimagelist";
    const QString URL_GET_NEURON_LIST = SERVER_IP + "/dynamic/ano/getneuronlist";
    const QString URL_GET_ANO_LIST = SERVER_IP + "/dynamic/ano/getanolist";
    const QString URL_DOWNLOAD_IMAGE = SERVER_IP + "/dynamic/image/cropimage";
    const QString URL_GET_BBSWC = SERVER_IP + "/dynamic/coll/getswcbb";
};

#endif // HTTPUTILSIMAGE_H
