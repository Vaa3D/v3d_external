#include "loadmanagewidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QString>
#include <qDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include <qjson/qobjecthelper.h>
#include <chrono>
#include "../control/CViewer.h"

QNetworkAccessManager* LoadManageWidget::accessManager= new QNetworkAccessManager();
QString LoadManageWidget::HostAddress="";
QString LoadManageWidget::DBMSAddress="";
QString LoadManageWidget::ApiVersion="";
QString LoadManageWidget::m_ano="";
QString LoadManageWidget::m_port="";
LoadManageWidget::LoadManageWidget(UserInfo *user): userinfo(user)
{
//    this->setAttribute(Qt::WA_DeleteOnClose);
//    getImageBtn=new QPushButton("GetImage",this);
//    getNeuronBtn=new QPushButton("GetNeuron",this);
//    getAnoBtn=new QPushButton("GetAno",this);
    loadBtn=new QPushButton("LoadAno",this);

//    imageWidget=new QListWidget(this);
//    neuronWidget=new QListWidget(this);
    anoWidget=new QListWidget(this);
    // 创建字体对象
    QFont font;
    // 设置字体大小
    font.setPointSize(12); // 设置字体大小为12点
    anoWidget->setFont(font);

    // 获取桌面信息
    QDesktopWidget desktopWidget;

    // 获取屏幕宽度和高度
    int screenWidth = desktopWidget.width();
    int screenHeight = desktopWidget.height();

    // 计算屏幕中央的坐标，略微向左移动
    int centerX = screenWidth / 2 - 50;
    int centerY = screenHeight / 2;

//    int currentScreen = qApp->desktop()->screenNumber(this);//程序所在的屏幕编号
//    QRect rect = qApp->desktop()->screenGeometry(currentScreen);//程序所在屏幕尺寸
//    anoWidget->move((rect.width() - anoWidget->width()) / 2, (rect.height() - anoWidget->height()) / 2);//移动到所在屏幕中间

    // 设置子部件的位置
//    anoWidget->setGeometry(centerX-anoWidget->minimumWidth()/2, centerY-anoWidget->minimumHeight()/2, anoWidget->minimumWidth(), anoWidget->minimumHeight());

//    QVBoxLayout *imageLayout,*neuronLayout,*anoLayout;

//    imageLayout=new QVBoxLayout;
//    imageLayout->addWidget(getImageBtn);
//    imageLayout->addWidget(imageWidget);

//    neuronLayout=new QVBoxLayout();
//    neuronLayout->addWidget(getNeuronBtn);
//    neuronLayout->addWidget(neuronWidget);


//    QHBoxLayout *listLayout=new QHBoxLayout();
//    listLayout->addLayout(imageLayout);
//    listLayout->addLayout(neuronLayout);
//    listLayout->addLayout(anoLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout(this);
    mainLayout->addWidget(anoWidget);
    mainLayout->addWidget(loadBtn);

    setLayout(mainLayout);
//    this->setMinimumSize(850, 250);
    int currentScreen = qApp->desktop()->screenNumber(this);//程序所在的屏幕编号
    QRect rect = qApp->desktop()->screenGeometry(currentScreen);//程序所在屏幕尺寸

    this->setFixedSize(900, 380);
    this->move((rect.width() - this->width()) / 2 - 50, (rect.height() - this->height()) / 2);//移动到所在屏幕中间
//    // 让父部件居中
//    this->setGeometry(centerX - this->width() / 2,
//                      centerY - this->height() / 2,
//                      this->width(),
//                      this->height());

//    connect(getImageBtn,SIGNAL(clicked()),this,SLOT(getImages()));
//    connect(getNeuronBtn,SIGNAL(clicked()),this,SLOT(getNeurons()));
//    connect(getAnoBtn,SIGNAL(clicked()),this,SLOT(getAnos()));
    connect(loadBtn,SIGNAL(clicked()),this,SLOT(loadAno()));
    getUserId();
    getAnos();
}

void LoadManageWidget::getUserId(){
    QNetworkRequest request;
    QString urlForGetUser = DBMSAddress + "/GetUserByName";
    request.setUrl(QUrl(urlForGetUser));
//    request.setUrl(QUrl("http://114.117.165.134:14252/proto.DBMS/GetUser"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("UserName",userinfo->name);
    userVerify.insert("UserPassword",userinfo->passwd);
    QVariantMap metaInfo;
    metaInfo.insert("ApiVersion",ApiVersion);
    QVariantMap param;
    param.insert("metaInfo",metaInfo);
    param.insert("UserVerifyInfo",userVerify);
    param.insert("UserName",userinfo->name);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getUserId "<<code;
    if(code==200)
    {
        json=reply->readAll();

        QJson::Parser parser;

        auto result = parser.parse(json,&ok).toMap();
        auto metaInfo = result["metaInfo"].toMap();
        bool status = metaInfo["Status"].toBool();
        QString message = metaInfo["Message"].toString();
        if(!status){
            QString msg = "GetUser Failed! " + message;
            qDebug()<<msg;
            QMessageBox::information(0,tr("Message "),
                                     tr(msg.toStdString().c_str()),
                                     QMessageBox::Ok);
            return;
        }

        auto userInfo = result["UserInfo"].toMap();
        userinfo->id = userInfo["UserId"].toInt();

    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}

void LoadManageWidget::getAnos(){
    QNetworkRequest request;
    QString urlForGetAllSwcMetaInfo = DBMSAddress + "/GetAllSwcMetaInfo";
    request.setUrl(QUrl(urlForGetAllSwcMetaInfo));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("UserName",userinfo->name);
    userVerify.insert("UserPassword",userinfo->passwd);
    QVariantMap metaInfo;
    metaInfo.insert("ApiVersion",ApiVersion);
    QVariantMap param;
    param.insert("metaInfo",metaInfo);
    param.insert("UserVerifyInfo",userVerify);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    anoWidget->clear();

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getAnos"<<code;
    if(code==200)
    {
        json=reply->readAll();

        QJson::Parser parser;
        auto result = parser.parse(json,&ok).toMap();
        auto metaInfo = result["metaInfo"].toMap();
        bool status = metaInfo["Status"].toBool();
        QString message = metaInfo["Message"].toString();
        if(!status){
            QString msg = "GetUser Failed! " + message;
            qDebug()<<msg;
            QMessageBox::information(0,tr("Message "),
                                     tr(msg.toStdString().c_str()),
                                     QMessageBox::Ok);
            return;
        }

        auto swcInfos = result["SwcInfo"].toList();
        std::vector<int> imageList;
        std::vector<int> indexs;
        for(int i=0; i<swcInfos.size(); i++){
            indexs.push_back(i);
        }
        for(auto &swcInfo:swcInfos){
            auto item = swcInfo.toMap();
            QStringList parts = item["Name"].toString().split("_");
            //只根据image前缀进行排序
            QString image_pre = parts[0];
            imageList.push_back(image_pre.toInt());
        }

        if(indexs.size()>=2){
            for(int i=1; i<indexs.size(); i++) {
                int temp = imageList[i];
                int tempIndex = indexs[i];
                int k = i-1;
                for(; k >= 0 && imageList[k] > temp; ) {
                    k--;
                }

                for(int j=i; j>k+1; j--) {
                    imageList[j] = imageList[j-1];
                    indexs[j] = indexs[j-1];
                }
                imageList[k+1] = temp;
                indexs[k+1] = tempIndex;
            }
        }

        for(int i=0; i<indexs.size(); i++){
            auto item = swcInfos.at(indexs[i]).toMap();
            int removedLen = QString(".ano.eswc").length();
            int len = item["Name"].toString().size();
            QString anoName = item["Name"].toString().remove(len-removedLen, removedLen);
//            QString anoName = item["Name"].toString();
            anoWidget->addItem(anoName);
        }
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}

void LoadManageWidget::loadAno()
{
    terafly::CViewer *cur_win = terafly::CViewer::getCurrent();
    if(!cur_win)
    {
        QMessageBox::information(this, tr("Error"),tr("please load the brain data."));
        return;
    }

    qDebug()<<"begin to load Ano";
    if(!anoWidget->currentItem()) return;

    QString anoName = anoWidget->currentItem()->text().trimmed();
    QStringList parts = anoName.split("_");
    QString image_pre = parts[0];
    QString image_suf = parts[1];
    QString image, neuron;
    //image是否带后缀
    if(image_suf.size() == 1){
        image = image_pre + "_" + image_suf;
        neuron = image + "_" + parts[2];
    }
    else{
        image = parts[0];
        neuron = parts[0] + "_" + parts[1];
    }

    QNetworkRequest request;
    request.setUrl(QUrl(HostAddress+"/collaborate/inheritother"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    qDebug()<<"userinfo->passwd: "<<userinfo->passwd;
    QVariantMap param;
    param.insert("image",image);
    param.insert("neuron",neuron);
    param.insert("ano",anoName);
    param.insert("user",userVerify);

    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    qDebug()<<"replycode_before"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"loadAno"<<code;
    if(code==200)
    {
        qDebug()<<"replycode"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        json=reply->readAll();
        qDebug()<<"loadAnojson"<<json;
        QJson::Parser parser;
        auto result=parser.parse(json,&ok).toMap();
        auto ano=result["ano"].toString();
        auto port=result["port"].toString();
        qDebug()<<ano<<"\n"<<port;
        m_ano = ano;
        m_port = port;
        emit Load(ano,port);
    }
    else if(code==400||code==401)
    {
        //qDebug()<<code;
        QMessageBox::information(0,tr("Message "),
                                 tr("Login error! Please check the username and the password."),
                                 QMessageBox::Ok);
    }

    else if(code==502||code==503||code==504)
    {
        //错误警告
        //qDebug()<<request.url()<<" 1\n"<<json<<" "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QMessageBox::information(0,tr("Message "),
                                 tr("Allocate port error!"),
                                 QMessageBox::Ok);
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}


