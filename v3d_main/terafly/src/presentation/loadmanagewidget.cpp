#include "loadmanagewidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QString>
#include <qDebug>
#include <chrono>
#include "../control/CViewer.h"
#include "json.hpp"

using json = nlohmann::json;

QNetworkAccessManager* LoadManageWidget::accessManager= new QNetworkAccessManager();
QString LoadManageWidget::HostAddress="";
QString LoadManageWidget::DBMSAddress="";
QString LoadManageWidget::ApiVersion="";
QString LoadManageWidget::m_port="";
QString LoadManageWidget::curSwcUuid="";
LoadManageWidget::LoadManageWidget(UserInfo *user): userinfo(user)
{
    // 创建字体对象
    QFont font;
    // 设置字体大小
    font.setPointSize(11); // 设置字体大小为11点
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *btnLayout = new QHBoxLayout(this);
    // 创建 QSplitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // 创建项目的 QListWidget
    projectWidget = new QListWidget(this);
    projectWidget->addItems(QStringList() << "Project 1" << "Project 2" << "Project 3");
    connect(projectWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(displayItems(QListWidgetItem*, QListWidgetItem*)));
    projectWidget->setFont(font);

    // 创建swc的 QListWidget
    swcWidget = new QListWidget(this);
    swcWidget->setFont(font);

    // 将两个 QListWidget 添加到 QSplitter
    splitter->addWidget(projectWidget);
    splitter->addWidget(swcWidget);

    // 设置初始尺寸比例
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    loadBtn=new QPushButton("LoadAno", this);
    reloadDataBtn=new QPushButton("ReloadData", this);

    btnLayout->addWidget(reloadDataBtn);
    btnLayout->addWidget(loadBtn);

    mainLayout->addWidget(splitter);
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);
    //    this->setMinimumSize(850, 250);
    // 获取屏幕尺寸
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    this->setMinimumSize(1100, 420);
    this->move((screenWidth - this->width()) / 2 - 50, (screenHeight - this->height()) / 2);//移动到所在屏幕中间

    connect(loadBtn,SIGNAL(clicked()),this,SLOT(loadAno()));
    connect(reloadDataBtn,SIGNAL(clicked()),this,SLOT(reloadData()));
    getUserId();
    getAllProjectSwcList();
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

    // 将 QVariantMap 转换为 QJsonObject
    QJsonObject jsonObject = QJsonObject::fromVariantMap(param);

    // 将 QJsonObject 序列化为 JSON 字符串
    QJsonDocument doc(jsonObject);
    QByteArray byteArray = doc.toJson();

    QNetworkReply* reply = accessManager->post(request, byteArray);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getUserId "<<code;
    if(code==200)
    {
        QByteArray tmpArray = reply->readAll();
        std::string jsonString = tmpArray.toStdString();
        json response_json = json::parse(jsonString);

        auto metaInfo = response_json["metaInfo"];
        bool status = metaInfo["Status"];
        QString message = QString::fromStdString(metaInfo["Message"]);
        if(!status){
            QString msg = "GetUser Failed! " + message;
            qDebug()<<msg;
            QMessageBox::information(0,tr("Message "),
                                     tr(msg.toStdString().c_str()),
                                     QMessageBox::Ok);
            return;
        }

        auto userInfo = response_json["UserInfo"];
        userinfo->id = userInfo["UserId"];

    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}

void LoadManageWidget::getAllProjectSwcList(){
    QNetworkRequest request;
    QString urlForGetAllProject = DBMSAddress + "/GetAllProject";
    request.setUrl(QUrl(urlForGetAllProject));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("UserName",userinfo->name);
    userVerify.insert("UserPassword",userinfo->passwd);
    QVariantMap metaInfo;
    metaInfo.insert("ApiVersion",ApiVersion);
    QVariantMap param;
    param.insert("metaInfo",metaInfo);
    param.insert("UserVerifyInfo",userVerify);

    // 将 QVariantMap 转换为 QJsonObject
    QJsonObject jsonObject = QJsonObject::fromVariantMap(param);

    // 将 QJsonObject 序列化为 JSON 字符串
    QJsonDocument doc(jsonObject);
    QByteArray byteArray = doc.toJson();

    QNetworkReply* reply = accessManager->post(request, byteArray);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    projectWidget->clear();
    swcWidget->clear();
    proName2SwcListMap.clear();
    proName2IdMap.clear();
    proAndSwcName2SwcIdMap.clear();

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getAllProject: "<<code;
    if(code==200)
    {
        QByteArray tmpArray = reply->readAll();
        std::string jsonString = tmpArray.toStdString();
        json response_json = json::parse(jsonString);

        auto metaInfo = response_json["metaInfo"];
        bool status = response_json["Status"];
        QString message = QString::fromStdString(metaInfo["Message"]);
        if(!status){
            QString msg = "GetAllProject Failed! " + message;
            qDebug()<<msg;
            QMessageBox::information(0,tr("Message "),
                                     tr(msg.toStdString().c_str()),
                                     QMessageBox::Ok);
            return;
        }

        auto projectInfos = response_json["ProjectInfo"];
        for(int i = 0; i < projectInfos.size(); ++i){
            auto projectInfo = projectInfos.at(i);
            QString projectName = QString::fromStdString(projectInfo["Name"]);
            QString proUuid = QString::fromStdString(projectInfo["Base"]["Uuid"]);
            proName2IdMap[projectName] = proUuid;
            projectWidget->addItem(projectName);
            getAllSwcUuidAndNameByProId(projectName, proUuid);
        }

        if(projectWidget->count() > 0){
            projectWidget->setCurrentRow(0);
        }
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}

void LoadManageWidget::getAllSwcUuidAndNameByProId(QString proName, QString proUuid){
    //    qDebug()<<proUuid;
    QNetworkRequest request;
    QString urlForGetAllProject = DBMSAddress + "/GetProjectSwcNamesByProjectUuid";
    request.setUrl(QUrl(urlForGetAllProject));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QVariantMap userVerify;
    userVerify.insert("UserName",userinfo->name);
    userVerify.insert("UserPassword",userinfo->passwd);
    QVariantMap metaInfo;
    metaInfo.insert("ApiVersion",ApiVersion);
    QVariantMap param;
    param.insert("metaInfo",metaInfo);
    param.insert("UserVerifyInfo",userVerify);
    param.insert("ProjectUuid",proUuid);

    // 将 QVariantMap 转换为 QJsonObject
    QJsonObject jsonObject = QJsonObject::fromVariantMap(param);

    // 将 QJsonObject 序列化为 JSON 字符串
    QJsonDocument doc(jsonObject);
    QByteArray byteArray = doc.toJson();

    QNetworkReply* reply = accessManager->post(request, byteArray);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(code != 200){
        qDebug()<<"getProjectSwcNamesByProjectUuid: "<<code;
    }
    if(code==200)
    {
        QByteArray tmpArray=reply->readAll();

        std::string jsonString = tmpArray.toStdString();
        json response_json = json::parse(jsonString);

        auto metaInfo = response_json["metaInfo"];
        bool status = metaInfo["Status"];
        QString message = QString::fromStdString(metaInfo["Message"]);
        if(!status){
            QString msg = "GetProjectSwcNamesByProjectUuid Failed! " + message;
            qDebug()<<msg;
            QMessageBox::information(0,tr("Message "),
                                     tr(msg.toStdString().c_str()),
                                     QMessageBox::Ok);
            return;
        }

        auto swcUuidNameList = response_json["swcUuidName"];
        std::vector<int> imageList;
        std::vector<int> indexs;
        QList<QString> swcNameList;
        QList<QString> sortedSwcNameList;

        for(int j = 0; j < swcUuidNameList.size(); ++j){
            indexs.push_back(j);
            QString swcName = QString::fromStdString(swcUuidNameList.at(j)["SwcName"]);
            QString swcUuid = QString::fromStdString(swcUuidNameList.at(j)["SwcUuid"]);

            int removedLen = QString(".ano.eswc").length();
            int len = swcName.size();
            swcName = swcName.remove(len-removedLen, removedLen);
            swcNameList.append(swcName);

            QString proAndSwcName = proName + "_" + swcName;
            proAndSwcName2SwcIdMap[proAndSwcName] = swcUuid;
        }

        for(auto &swcName : swcNameList){
            QStringList parts = swcName.split("_");
            //只根据image前缀进行排序
            QString image_pre = parts[0];
            imageList.push_back(image_pre.toInt());
        }

        if(indexs.size()>=2){
            for(int j = 1; j < indexs.size(); ++j) {
                int temp = imageList[j];
                int tempIndex = indexs[j];
                int k = j - 1;
                for(; k >= 0 && imageList[k] > temp; ) {
                    k--;
                }

                for(int l = j; l > k+1; --l) {
                    imageList[l] = imageList[l-1];
                    indexs[l] = indexs[l-1];
                }
                imageList[k+1] = temp;
                indexs[k+1] = tempIndex;
            }
        }

        for(int j = 0; j < indexs.size(); ++j){
            QString swcName = swcNameList.at(indexs[j]);
            sortedSwcNameList.append(swcName);
        }
        proName2SwcListMap[proName] = sortedSwcNameList;
    }
    else{
        QString reason=reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0,tr("Message "),
                                 tr(reason.toStdString().c_str()),
                                 QMessageBox::Ok);
    }
}

void LoadManageWidget::displayItems(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current == nullptr || current->text() == ""){
        return;
    }
    swcWidget->clear();
    if(proName2SwcListMap.find(current->text()) == proName2SwcListMap.end()){
        return;
    }
    QList<QString> swcList = proName2SwcListMap.at(current->text());
    for(auto swcName : swcList){
        swcWidget->addItem(swcName);
    }
}

void LoadManageWidget::reloadData(){
    getUserId();
    getAllProjectSwcList();
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
    if(!projectWidget->currentItem() || !swcWidget->currentItem()) return;

    QString projectName = projectWidget->currentItem()->text().trimmed();
    QString swcName = swcWidget->currentItem()->text().trimmed();
    QString proAndSwcName = projectName + "_" + swcName;
    curSwcUuid = proAndSwcName2SwcIdMap[proAndSwcName];

    QStringList parts = swcName.split("_");
    QString image_pre = parts[0];
    QString image_suf = parts[1];
    QString image, neuron;
    //image是否带后缀
    if(image_suf.size() == 1){
        image = parts[0] + "_" + parts[1];
        neuron = image + "_" + parts[2];
    }
    else{
        image = parts[0];
        neuron = image + "_" + parts[1];
    }

    QNetworkRequest request;
    request.setUrl(QUrl(HostAddress+"/collaborate/inheritother"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QVariantMap userVerify;
    userVerify.insert("name",userinfo->name);
    userVerify.insert("passwd",userinfo->passwd);
    qDebug()<<"userinfo->passwd: "<<userinfo->passwd;
    QVariantMap param;
    param.insert("project",projectName);
    param.insert("image",image);
    param.insert("neuron",neuron);
    param.insert("ano",swcName);
    param.insert("user",userVerify);

    // 将 QVariantMap 转换为 QJsonObject
    QJsonObject jsonObject = QJsonObject::fromVariantMap(param);

    // 将 QJsonObject 序列化为 JSON 字符串
    QJsonDocument doc(jsonObject);
    QByteArray byteArray = doc.toJson();

    QNetworkReply* reply = accessManager->post(request, byteArray);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    qDebug()<<"replycode_before"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"loadAno"<<code;
    if(code==200)
    {
        qDebug()<<"replycode"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QByteArray tmpArray=reply->readAll();

        std::string jsonString = tmpArray.toStdString();
        json response_json = json::parse(jsonString);

        auto ano=QString::fromStdString(response_json["ano"]);
        auto port=QString::fromStdString(response_json["port"]);
        qDebug()<<ano<<"\n"<<port;
        m_port = port;
        emit triggerStartCollaborate(port);
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
