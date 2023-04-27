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

QNetworkAccessManager* LoadManageWidget::accessManager= new QNetworkAccessManager();
QString LoadManageWidget::HostAddress="http://192.168.3.155:8000/dynamic";
LoadManageWidget::LoadManageWidget(UserInfo *user): userinfo(user)
{
//    this->setAttribute(Qt::WA_DeleteOnClose);
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
    qDebug()<<"enter getImages";
    QNetworkRequest request;
    request.setUrl(QUrl(HostAddress+"/collaborate/getanoimage"));
    qDebug()<<"1";
    qDebug()<<HostAddress;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    qDebug()<<"2";
    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    qDebug()<<"3";
    QVariantMap param;
    param.insert("user",userVerify);
    qDebug()<<"4";
    QJson::Serializer serializer;
    bool ok;
    QByteArray json=serializer.serialize(param,&ok);
    qDebug()<<"5";
    qDebug()<<json;
    QNetworkReply* reply = accessManager->post(request, json);
    if(!reply)
        qDebug()<<"reply = nullptr";

    qDebug()<<"6";
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    imageWidget->clear();
    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getImages"<<code;
    if(code==200)
    {
        //qDebug()<<"200 Ok";
        QStringList imageList;
        for(auto &image: QString(reply->readAll()).split(','))
        {
            imageList.append(image);
        }
        imageList.removeDuplicates();
        imageList.removeAll("182722,191797,191798,191799,192346,192348,194060,18454");
        imageWidget->addItems(imageList);
    }
    else if(code==400||code==401)
    {
        //qDebug()<<code;
        QMessageBox::information(0,tr("Message "),
                                 tr("Login error! Please check the username and the password."),
                                 QMessageBox::Ok);
    }
    else if(code==500)
    {
        //错误警告
        //qDebug()<<request.url()<<" 1\n"<<json<<" "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QMessageBox::information(0,tr("Message "),
                                 tr("Database error!"),
                                 QMessageBox::Ok);
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
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
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    //connect(accessManager, SIGNAL(finished(QNetworkReply*)),&eventLoop,SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    neuronWidget->clear();

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getNeurons"<<code;
    if(code==200)
    {
        json=reply->readAll();
        qDebug()<<"getNeuronsjson"<<json;

        QJson::Parser parser;
        auto neurons=parser.parse(json,&ok).toList();
        for(auto &neuron:neurons){
            auto item=neuron.toMap();
            neuronWidget->addItem(item["name"].toString());
        }

    }
    else if(code==400||code==401)
    {
        //qDebug()<<code;
        QMessageBox::information(0,tr("Message "),
                                 tr("Login error! Please check the username and the password."),
                                 QMessageBox::Ok);
    }
    else if(code==500)
    {
        //错误警告
        //qDebug()<<request.url()<<" 1\n"<<json<<" "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QMessageBox::information(0,tr("Message "),
                                 tr("Database error!"),
                                 QMessageBox::Ok);
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
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
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    anoWidget->clear();

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getAnos"<<code;
    if(code==200)
    {
        // id是数据库TUserinfo表的自增主键Id
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

    }
    else if(code==400||code==401)
    {
        //qDebug()<<code;
        QMessageBox::information(0,tr("Message "),
                                 tr("Login error! Please check the username and the password."),
                                 QMessageBox::Ok);
    }
    else if(code==500)
    {
        //错误警告
        //qDebug()<<request.url()<<" 1\n"<<json<<" "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QMessageBox::information(0,tr("Message "),
                                 tr("Database error!"),
                                 QMessageBox::Ok);
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
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
        emit signal(4);
    }
}


