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
    userlogin = new HttpUtilsUser(this);
    connect(userlogin, SIGNAL(loginSuccess()), this, SLOT(emitShowMain()));
    connect(userlogin, SIGNAL(loginFailed()), this, SLOT(doLoginFailed()));
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

    userlogin->loginWithHttp(userInfo);
}

void LoginDialog::emitShowMain()
{
    this->hide();
    emit showMain();
}

void LoginDialog::doLoginFailed()
{
    QMessageBox::critical(this, "ERROR!", "Log in failed");
}



