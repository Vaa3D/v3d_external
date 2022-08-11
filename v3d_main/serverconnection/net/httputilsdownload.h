#ifndef HTTPUTILSDOWNLOAD_H
#define HTTPUTILSDOWNLOAD_H

#include "net/httputils.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QThread>
class HttpUtilsDownLoad: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsDownLoad(QObject *parent = nullptr);
    ~HttpUtilsDownLoad();

    void downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);
    void getSWCWithHttp(QString arborname,QString res, float offsetX, float offsetY, float offsetZ, int size);
    virtual void asyncPostRequest(QString url, QJsonObject &body);
    void bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);
    void Download(QString brainId, QString arborname,QString res, int offsetX, int offsetY, int offsetZ, int size);
    QByteArray coordinateconvert(QString response);
signals:
    //pass
//    void todelete();
public slots:
    void downloadReplyFinished(QNetworkReply* reply);
    void SWCReply(QNetworkReply* reply);
    void deletethis();
    void istimeout();
private:



    QString URL_DOWNLOAD_IMAGE = SERVER_IP + "/dynamic/image/cropimage";
    QString URL_GET_SWC = SERVER_IP + "/dynamic/swc/cropswc";
    QNetworkAccessManager *managerimg;
    QNetworkAccessManager *managerswc;
    QString mbrainId;
    QString mres;
    int moffsetX, moffsetY, moffsetZ;
    float start_x,start_y,start_z;
    bool isswcdone,isimgdone;
    bool ttd;
    QElapsedTimer *m_etimer;
    QTimer *m_timer;
};

#endif // HTTPUTILSDOWNLOAD_H
