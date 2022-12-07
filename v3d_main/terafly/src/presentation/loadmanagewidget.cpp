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

QString LoadManageWidget::HostAddress="http://192.168.3.155:8000/dynamic";
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
    request.setUrl(QUrl(HostAddress+"/collaborate/getanoimage"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    QVariantMap param;
    param.insert("user",userVerify);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);

    QNetworkReply* reply = accessManager->post(request, json);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    imageWidget->clear();
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {
        qDebug()<<"200 Ok";
        QStringList imageList;
        for(auto &image: QString(reply->readAll()).split(','))
        {
            imageList.append(image);
        }
        imageList.removeDuplicates();
        imageList.removeAll("182722,191797,191798,191799,192346,192348,194060,18454");
        imageWidget->addItems(imageList);
    }else{
        //错误警告
        qDebug()<<request.url()<<" 1\n"<<json<<" "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit signal(1);
    }
}
void LoadManageWidget::getNeurons()
{
    if(!imageWidget->currentItem()) return;

    QNetworkRequest request;
    request.setUrl(QUrl(HostAddress+"/collaborate/getanoneuron"));
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
        qDebug()<<"getNeuronsjson"<<json;

        QJson::Parser parser;
        auto neurons=parser.parse(json,&ok).toList();
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
    request.setUrl(QUrl(HostAddress+"/collaborate/getano"));
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
        userinfo->id=reply->rawHeader("Set-Cookie").toInt();
        qDebug()<<"user info "<<reply->rawHeader("Set-Cookie")<<" "<<userinfo->id;
        json=reply->readAll();
        qDebug()<<json<<"getAnos";
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
    qDebug()<<"begin to load Ano";
    if(!anoWidget->currentItem()) return;
    QNetworkRequest request;
    request.setUrl(QUrl(HostAddress+"/collaborate/inheritother"));
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

    qDebug()<<"replycode_before"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200)
    {
        qDebug()<<"replycode"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        json=reply->readAll();
        qDebug()<<"loadAnojson"<<json;
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

