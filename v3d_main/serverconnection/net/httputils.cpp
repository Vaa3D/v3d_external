#include "httputils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QMessageBox> // delete this will report QOBject error

HttpUtils::HttpUtils(QWidget *parent)
    : QObject(parent)
{


}

HttpUtils::~HttpUtils()
{

}

void HttpUtils::asyncPostRequest(QString url, QJsonObject &body)
{
    Q_UNUSED(url);
    Q_UNUSED(body);
}
