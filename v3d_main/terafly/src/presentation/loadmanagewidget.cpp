#include "loadmanagewidget.h"
#include <QHBoxLayout>
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

const QString address="http://192.168.3.155:8000/dynamic";
LoadManageWidget::LoadManageWidget(QNetworkAccessManager *accessManager,UserInfo *user):accessManager(accessManager),userinfo(user)
{
    getImageBtn=new QPushButton("GetImage",this);
    getNeuronBtn=new QPushButton("GetNeuron",this);
    getAnoBtn=new QPushButton("GetAno",this);
    loadBtn=new QPushButton("LoadAno",this);

    imageWidget=new QListWidget(this);
    neuronWidget=new QListWidget(this);
    anoWidget=new QListWidget(this);

    QVBoxLayout *imageLayout,*neuronLayout,*anoLayout;
    imageLayout=new QVBoxLayout;
    imageLayout->addWidget(getImageBtn);
    imageLayout->addWidget(imageWidget);

    neuronLayout=new QVBoxLayout();
    neuronLayout->addWidget(getNeuronBtn);
    neuronLayout->addWidget(neuronWidget);

    anoLayout=new QVBoxLayout();
    anoLayout->addWidget(getAnoBtn);
    anoLayout->addWidget(anoWidget);

    QHBoxLayout *listLayout=new QHBoxLayout();
    listLayout->addLayout(imageLayout);
    listLayout->addLayout(neuronLayout);
    listLayout->addLayout(anoLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout();
    mainLayout->addLayout(listLayout);
    mainLayout->addWidget(loadBtn);

    setLayout(mainLayout);

    connect(getImageBtn,SIGNAL(clicked()),this,SLOT(getImages()));
    connect(getNeuronBtn,SIGNAL(clicked()),this,SLOT(getNeurons()));
    connect(getAnoBtn,SIGNAL(clicked()),this,SLOT(getAnos()));
    connect(loadBtn,SIGNAL(clicked()),this,SLOT(loadAno()));

}


void LoadManageWidget::getImages()
{
    QNetworkRequest request;
    request.setUrl(QUrl(address+"/collaborate/getanoimage"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(userVerify,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    imageWidget->clear();
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {

        for(auto &image: QString(reply->readAll()).split(',')){
            imageWidget->addItem(image);
        }
    }else{
        //错误警告
        emit signal(1);
    }
}
void LoadManageWidget::getNeurons()
{
    if(!imageWidget->currentItem()) return;

    QNetworkRequest request;
    request.setUrl(QUrl(address+"/collaborate/getanoneuron"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);

    QVariantMap param;

    param.insert("image",imageWidget->currentItem()->text().trimmed());

    param.insert("user",userVerify);

    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
     neuronWidget->clear();
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {
        json=reply->readAll();

        QJson::Parser parser;
        auto neurons=parser.parse(json,&ok).toList();
        qDebug()<<neurons;
        for(auto &neuron:neurons){
            auto item=neuron.toMap();
            neuronWidget->addItem(item["name"].toString());
        }

    }else{
        //错误警告
        emit signal(2);
    }
}
void LoadManageWidget::getAnos()
{
    if(!neuronWidget->currentItem()) return;
    QNetworkRequest request;
    request.setUrl(QUrl(address+"/collaborate/getano"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    QVariantMap param;
    param.insert("neuron",neuronWidget->currentItem()->text().trimmed());
    param.insert("user",userVerify);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
   QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    anoWidget->clear();
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {
        json=reply->readAll();
        QJson::Parser parser;
        auto neurons=parser.parse(json,&ok).toList();
        for(auto &neuron:neurons){
            auto item=neuron.toMap();
            anoWidget->addItem(item["name"].toString());
        }

    }else{
        //错误警告
        emit signal(3);
    }
}

void LoadManageWidget::loadAno()
{
    if(!anoWidget->currentItem()) return;
    QNetworkRequest request;
    request.setUrl(QUrl(address+"/collaborate/inheritother"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    QVariantMap param;
    param.insert("image",imageWidget->currentItem()->text().trimmed());
    param.insert("neuron",neuronWidget->currentItem()->text().trimmed());
    param.insert("ano",anoWidget->currentItem()->text().trimmed());
    param.insert("user",userVerify);

    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {
        json=reply->readAll();
        QJson::Parser parser;
        auto result=parser.parse(json,&ok).toMap();
        auto ano=result["ano"].toString();
        auto port=result["port"].toString();
        qDebug()<<ano<<"\n"<<port;
        emit Load(ano,port);
    }else{
        emit signal(4);
    }
}


