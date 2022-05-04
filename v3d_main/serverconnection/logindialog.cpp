#include "logindialog.h"
#include "ui_logindialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkRequest>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setMinimumSize(QSize(420, 180));
    setWindowTitle(tr("Log in"));
//    manager = new QNetworkAccessManager(this);
    connect(&uesrlogin, SIGNAL(loginSuccess()), this, SLOT(emitShowMain()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}


/**
 * @brief slot, call in confirmButton clicked
 */
void LoginDialog::on_confirmButton_clicked()
{
    // get Username and password
    QString username = ui->userLineEdit->text();
    QString password = ui->pwdLineEdit->text();

    //    startRequest(QUrl("http://139.155.28.154:26000/dynamic/user/login"));
//    QString baseUrl = "http://139.155.28.154:26000/dynamic/user/login";
//    QUrl url(baseUrl);

    // 构造Json数据
    QJsonObject userInfo;
    userInfo.insert("name", username);
    userInfo.insert("passwd", password);
//    user.insert("user", userInfo);

    uesrlogin.loginWithHttp(userInfo);
}

void LoginDialog::emitShowMain()
{
    this->hide();
    emit showMain();
}

//void LoginDialog::replyFinished(QNetworkReply *reply)
//{
//    if(reply->error()) {
//        qDebug()<<reply->errorString();
//        QMessageBox::critical(this, "ERROR!", "Login failed");
//        reply->deleteLater();
//        return;
//    }
//    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    //        qDebug() << "statusCode: "<< statusCode;
//    if(statusCode == 200) {
//        this->hide();
//        emit showMain();
//    }

//}




