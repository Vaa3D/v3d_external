#ifndef CHECKMAPWIDGET_H
#define CHECKMAPWIDGET_H

#include <QNetworkReply>
#include <QWidget>
#include "serverconnection/infocache.h"


namespace Ui {
class CheckMapWidget;
}

class CheckMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CheckMapWidget(QWidget *parent = nullptr);
    ~CheckMapWidget();

//    void getBrainList();
    void downloadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);

public slots:
    void downloadImageFinish(QNetworkReply *reply);

private:
    Ui::CheckMapWidget *ui;

    const QString SERVER_IP  = "http://139.155.28.154:26000";
    const QString URL_DOWNLOAD_IMAGE = SERVER_IP + "/dynamic/image/cropimage";
};

#endif // CHECKMAPWIDGET_H
