#ifndef LOADMANAGEWIDGET_H
#define LOADMANAGEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
struct UserInfo{
    QString name,passwd;
};
class LoadManageWidget:public QWidget
{
    Q_OBJECT
public:
    LoadManageWidget(QNetworkAccessManager *accessManager,UserInfo *user);
    QPushButton *getImageBtn,*getNeuronBtn,*getAnoBtn,*loadBtn;
    QListWidget *imageWidget,*neuronWidget,*anoWidget;
    QNetworkAccessManager *accessManager;
    UserInfo *userinfo;
public slots:
    void getImages();
    void getNeurons();
    void getAnos();
    void loadAno();
signals:
    void Load(QString ano,QString port);
    void signal(int);
};



#endif // LOADMANAGEWIDGET_H
