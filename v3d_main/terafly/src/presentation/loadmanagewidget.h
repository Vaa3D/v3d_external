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
    QListWidget *swcWidget;
    QListWidget *projectWidget;
//    QPushButton *getImageBtn,*getNeuronBtn,*getAnoBtn,*loadBtn;
//    QListWidget *imageWidget,*neuronWidget,*anoWidget;
    static QNetworkAccessManager *accessManager;
    static QString m_port;
    UserInfo *userinfo;
    std::map<QString, QString> proName2IdMap;
    std::map<QString, QList<QString>> proName2SwcListMap;
    std::map<QString, QString> proAndSwcName2SwcIdMap;
    static QString curSwcUuid;
    static QString HostAddress;
    static QString DBMSAddress;
    static QString ApiVersion;
public slots:
//    void getImages();
//    void getNeurons();
    void getUserId();
    void getAllProjectSwcList();
    void getAllSwcUuidAndNameByProId(QString proName, QString proUuid);
    void loadAno();
    void displayItems(QListWidgetItem *current, QListWidgetItem *previous);
signals:
    void triggerStartCollaborate(QString port);
    void signal(int);
};



#endif // LOADMANAGEWIDGET_H
