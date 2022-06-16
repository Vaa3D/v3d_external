#include "checkmapwidget.h"
#include "ui_checkmapwidget.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include "serverconnection/net/networkutils.h"
#include <fstream>
/**
 * @brief
 * @param parent
 */
CheckMapWidget::CheckMapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CheckMapWidget)
{
    ui->setupUi(this);
//    connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this, &CheckMapWidget::downloadImageFinish);

}

CheckMapWidget::~CheckMapWidget()
{
    delete ui;
    //    delete potentialSomaInfo;
}

void CheckMapWidget::downloadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    QJsonObject pa1;
    pa1.insert("x", offsetX/2 - size/2);
    pa1.insert("y", offsetY/2 - size/2);
    pa1.insert("z", offsetZ/2 - size/2);

    QJsonObject pa2;
    pa2.insert("x", offsetX/2 + size/2);
    pa2.insert("y", offsetY/2 + size/2);
    pa2.insert("z", offsetZ/2 + size/2);

    QJsonObject bBox;
    bBox.insert("pa1", pa1);
    bBox.insert("pa2", pa2);
    bBox.insert("res", res);
    bBox.insert("obj", brainId);

    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());

    // requestBody
    QJsonObject body;
    body.insert("bb", bBox);
    body.insert("user", userInfo);

    // post request
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;

    dataArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "download image post json content: " << dataArray;

    QNetworkReply* downloadImage = NetWorkUtil::instance()->postRequst(URL_DOWNLOAD_IMAGE, body);
    Q_UNUSED(downloadImage);
}

//V3dR_GLWidget *CheckMapWidget::getGLWidget()
//{
//    return this->csglwidget;
//}

void CheckMapWidget::downloadImageFinish(QNetworkReply *reply)
{
    qDebug() << "do download image in checkmap";
    QByteArray response = reply->readAll();
//    qDebug() << response;
    // save file
//    QString storePath = QCoreApplication::applicationDirPath() + "/Image";
//    // brainId
//    QString fileName = brainId + "_" + res + "_" + offsetX + "_" + offsetY + "_" + offsetZ + ".v3dpbd";

    QString filePath = "c:\\Users\\SEU\\Desktop\\1.v3dpbd";
    QFile file(filePath);
    file.open(QFile::WriteOnly);
    file.write(response);
    file.close();

    // todo: render this image in CSMainwindow
//    csglwidget->loadObjectFromFile(filePath);
    qDebug() << "done download!";
    reply->deleteLater();
    reply = nullptr;
}
