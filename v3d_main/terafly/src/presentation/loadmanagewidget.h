#ifndef LOADMANAGEWIDGET_H
#define LOADMANAGEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QMessageBox>

struct UserInfo{
    QString name,passwd;
    int id;
    int colorid;
};
class LoadManageWidget:public QWidget
{
    Q_OBJECT
public:
    LoadManageWidget(UserInfo *user);
    QPushButton *loadBtn;
    QListWidget *anoWidget;
//    QPushButton *getImageBtn,*getNeuronBtn,*getAnoBtn,*loadBtn;
//    QListWidget *imageWidget,*neuronWidget,*anoWidget;
    static QNetworkAccessManager *accessManager;
    static QString m_ano;
    static QString m_port;
    UserInfo *userinfo;
    static QString HostAddress;
    static QString DBMSAddress;
    static QString ApiVersion;
public slots:
//    void getImages();
//    void getNeurons();
    void getUserId();
    void getAnos();
    void loadAno();
signals:
    void Load(QString ano,QString port);
    void signal(int);
};



#endif // LOADMANAGEWIDGET_H
