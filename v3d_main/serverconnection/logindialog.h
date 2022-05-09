#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

//#include "selectmodulewgt.h"
#include "net/httputilsuser.h"
/**
 *@Server IP: string SERVER_IP = "http://139.155.28.154:26000"
 */

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

//    SelectModuleWgt moduleWidget;
signals:
    void showMain();


private slots:

    void on_confirmButton_clicked();
    void emitShowMain();
    void doLoginFailed();

private:
    Ui::LoginDialog *ui;

    HttpUtilsUser *userlogin;

};
#endif // LOGINDIALOG_H
