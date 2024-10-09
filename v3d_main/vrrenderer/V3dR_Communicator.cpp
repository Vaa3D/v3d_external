#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include "3drenderer/v3dr_qualitycontroldialog.h"
#include <QRegExp>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>

//消息的拼接方式
//每个指令之间使用‘;’
//指令内部:指令头+数据体，使用','
//每个点使用‘’

QTcpSocket* V3dR_Communicator::socket=0;
V3dr_qualitycontrolDialog* V3dR_Communicator::qcDialog=0;
V3dr_onlineusersDialog* V3dR_Communicator::onlineUserDialog=0;
QString V3dR_Communicator::userId="";
QString V3dR_Communicator::userName = "";
QString V3dR_Communicator::password = "";
V3dR_Communicator::V3dR_Communicator(QObject *partent):b_isConnectedState(false), b_isWarnMulBifurcationHandled(false), b_isWarnLoopHandled(false), reconnectCnt(0), initConnectCnt(0), QObject(partent)
{
    CreatorMarkerPos = 0;
    CreatorMarkerRes = 0;
    userId="";
    userName="";
    //    qDebug()<<"userName=\"\"";

    //    socket = new QTcpSocket(this);
    resetdatatype();
    connect(this,SIGNAL(msgtoprocess(QString)),this,SLOT(TFProcess(QString)));
    connect(this,SIGNAL(msgtowarn(QString)),this,SLOT(processWarnMsg(QString)));
    connect(this,SIGNAL(msgtoanalyze(QString)),this,SLOT(processAnalyzeMsg(QString)));
    connect(this,SIGNAL(msgtosend(QString)),this,SLOT(processSendMsg(QString)));
    //    connect(this->socket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
    //    m_timerConnect = new QTimer(this);
    //    timer_iniconn=new QTimer(this);
//    timer_exit=new QTimer(this);
}

void V3dR_Communicator::onReadyRead()
{
    while(1){
        qDebug() << "enter onReadyRead " << socket->bytesAvailable() << " datatype.datasize " << datatype.datasize;
        if(!datatype.isFile)
        {
            //不是准备接受文件数据
            if(datatype.datasize==0){
                //准备接收数据头
                if(socket->canReadLine()){
                    QString msg=socket->readLine(1024).trimmed();
                    if(!msg.startsWith("DataTypeWithSize:")){
                        //                        socket->write({"Socket Receive ERROR!"});
                        //                        std::cerr<<userName.toStdString()+" receive not match format\n";
                        qDebug() << userId << " receive not match format\n";
                        socket->disconnectFromHost();
                    }
                    auto ps=msg.right(msg.size()-QString("DataTypeWithSize:").size()).split(' ');
                    datatype.isFile=ps[0].toUInt();
                    datatype.datasize=ps[1].toUInt();
                    if(datatype.isFile)
                        datatype.filesize=ps[2].toUInt();
                }else{
                    qDebug()<<"socket readline failed";
                    break;
                }
            }else{
                //已经接收了头，正在准备接收 消息数据
                if(socket->bytesAvailable()>=datatype.datasize){
                    char *data=new char[datatype.datasize+1];
                    socket->read(data,datatype.datasize);
                    data[datatype.datasize]='\0';
                    if(strlen(data)<128)
                        std::cout<<QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ").toStdString()<<" receive from "<<userId.toStdString()<<" :"<<data<<std::endl;
                    //处理消息
                    preprocessmsgs(QString(data).trimmed().split(';',QString::SkipEmptyParts));
                    resetdatatype();
                    delete [] data;

                }else{
                    qDebug()<<"socket->bytesAvailable<datatype datasize"<<"datatype.datasize "<<datatype.datasize;
                    break;
                }
            }
        }else{
            //已经接收了头，数据
            if(socket->bytesAvailable()>=datatype.datasize+datatype.filesize){
                char *data=new char[datatype.datasize+1];
                socket->read(data,datatype.datasize);
                data[datatype.datasize]='\0';
                // std::cout<<QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ").toStdString()<<(++receivedcnt)<<" receive from "<<username.toStdString()<<" :"<<data<<std::endl;
                //只会接收刚开始协作时同步的文件，开始加载同步数据由服务器发出消息通知
                char *filedata=new char[datatype.filesize];
                QDir dir(QCoreApplication::applicationDirPath()+"/loaddata");
                if(!dir.exists()){
                    dir.mkdir(QCoreApplication::applicationDirPath()+"/loaddata");
                }
                QFile f(QCoreApplication::applicationDirPath()+"/loaddata/"+data);
                socket->read(filedata,datatype.filesize);
                if(f.open(QIODevice::WriteOnly)){
                    f.write(filedata,datatype.filesize);
                }

                delete [] filedata;
                delete [] data;

                resetdatatype();
            }else{
                break;
            }
        }
    }
}

void V3dR_Communicator::sendMsg(QString msg)
{
    if(msg.size()<128)
        qDebug()<<"send:"<<msg;
    const std::string data=msg.toStdString();
    const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
    socket->write(header.c_str(),header.size());
    socket->write(data.c_str(),data.size());
    socket->flush();
}

void V3dR_Communicator::preprocessmsgs(QStringList list)
{
    //1. 开始协作
    //2. 更新用户
    //3. 处理协作指令
    qDebug()<<"begin to preprocessmsgs";

//    QRegExp usersRex("^/activeusers:(.*)$");
    QRegExp onlineUsersRex("^/onlineusers:(.*)$");
    QRegExp warnRex("^/WARN_(.*):.*$");
    QRegExp analyzeRex("^/FEEDBACK_ANALYZE_(.*):.*$");
    QRegExp defineRex("^/FEEDBACK_DEFINE_(.*):.*$");
    QRegExp sendRex("^/FEEDBACK_SEND_(.*):.*$");

    for(auto &msg:list)
    {
        if(msg.size()<64)
            qDebug()<<"OnRead:"<<msg;
        if(msg.startsWith("STARTCOLLABORATE:")){
            //            qDebug()<<"start collaborate_msg____Debug_zll"<<msg;
            emit load(msg.right(msg.size()-QString("STARTCOLLABORATE:").size()));
        }else if(msg.startsWith("/ACK")){
            emit ack();
        }
        else if(onlineUsersRex.indexIn(msg) != -1){
//            emit updateuserview(usersRex.cap(1));
            emit updateOnlineUsers(onlineUsersRex.cap(1));
        }else if(warnRex.indexIn(msg) != -1){
            emit msgtowarn(msg);
        }else if(analyzeRex.indexIn(msg) != -1 || defineRex.indexIn(msg) != -1){
            emit msgtoanalyze(msg);
        }else if(sendRex.indexIn(msg) != -1){
            emit msgtosend(msg);
        }
        else{
            emit msgtoprocess(msg);
        }
    }
}

void V3dR_Communicator::processWarnMsg(QString line){
    QRegExp warnreg("/WARN_(.*):(.*)");
    line=line.trimmed();
    if(warnreg.indexIn(line)!=-1)
    {
        QString reason=warnreg.cap(1).trimmed();
        QString msg=warnreg.cap(2).trimmed();

        if(reason=="ReloadFile"){
            if(msg=="outbound marker"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("Points out of the image boundary have occurred. The annotation file will be reload from the server to ensure synchronization."),
                                         QMessageBox::Ok);
            }
            if(msg=="outbound swcnode"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("Points out of the image boundary have occurred. The annotation file will be reload from the server to ensure synchronization."),
                                         QMessageBox::Ok);
            }
            if(msg=="cannot find connect seg"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("The server has detected an error. The annotation file will be reload from the server to ensure synchronization."),
                                         QMessageBox::Ok);
            }
            emit reloadFile(LoadManageWidget::m_port);
            return;
        }

        QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
        //        if(listwithheader.size()<2)
        //        {
        //            qDebug()<<"msg only contains header:"<<msg;
        //            return;
        //        }

        QString header = listwithheader[0];
        QString sender=header.split(" ").at(0).trimmed();
        listwithheader.removeAt(0);
        QString comment="";

        if(sender=="server"){
            if(reason=="TipUndone" || reason=="CrossingError" || reason=="MulBifurcation" || reason=="BranchingError")
            {

                if(reason == "TipUndone"){
                    comment = "Missing";
                }else if(reason == "CrossingError"){
                    comment = "Crossing error";
                }else if(reason == "MulBifurcation"){
                    comment = "Multifurcation";
                }else if(reason == "BranchingError"){
                    comment = "Branching error";
                }
                emit addManyMarkers(listwithheader.join(","), comment);

                emit updateQcInfo();
                emit updateQcMarkersCounts();
            }
            else if(reason=="Loop")
            {
                int result = header.split(" ").at(1).trimmed().toUInt();
                if(result == 1){
                    //                emit setDefineSomaActionState(true);
                }
                if(result == 0){
                    //                emit setDefineSomaActionState(false);
                    //                    QEventLoop eventLoop;
                    //                    connect(this, SIGNAL(addManyMarkersDone()), &eventLoop, SLOT(quit()));
                    emit addManyMarkers(listwithheader.join(","), "Loop");
                    //                    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

                    emit updateQcInfo();
                    emit updateQcMarkersCounts();
                }
            }
            else if(reason=="Approaching bifurcation"){
                emit addMarker(listwithheader[0], "Approaching bifurcation");

                emit updateQcInfo();
                emit updateQcMarkersCounts();
            }
            else if(reason=="DisconnectError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("Disconnect from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetSwcMetaInfoError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetSwcMetaInfoError from DBMS!"),
                                         QMessageBox::Ok);
//                emit reloadFile(LoadManageWidget::m_ano, LoadManageWidget::m_port);
            }
            else if(reason=="ApoFileNotFoundError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ApoFileNotFoundError!"),
                                         QMessageBox::Ok);
//                emit reloadFile(LoadManageWidget::m_ano, LoadManageWidget::m_port);
            }
            else if(reason=="GetApoDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetApoDataError from DBMS!"),
                                         QMessageBox::Ok);
//                emit reloadFile(LoadManageWidget::m_ano, LoadManageWidget::m_port);
            }
            else if(reason=="GetSwcFullNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetSwcFullNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
//                emit reloadFile(LoadManageWidget::m_ano, LoadManageWidget::m_port);
            }
            else if(reason=="AddSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("AddSwcNodeDataError from DBMS! Please try to reload the swcfile."),
                                         QMessageBox::Ok);
                emit reloadFile(LoadManageWidget::m_port);
            }
            else if(reason=="DeleteSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("DeleteSwcNodeDataError from DBMS! Please try to reload the swcfile."),
                                         QMessageBox::Ok);
                emit reloadFile(LoadManageWidget::m_port);
            }
            else if(reason=="ModifySwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ModifySwcNodeDataError from DBMS! Please try to reload the swcfile."),
                                         QMessageBox::Ok);
                emit reloadFile(LoadManageWidget::m_port);
            }
            else if(reason=="UpdateSwcAttachmentApoError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("UpdateSwcAttachmentApoError from DBMS! Please try to reload the swcfile."),
                                         QMessageBox::Ok);
//                emit reloadFile(LoadManageWidget::m_ano, LoadManageWidget::m_port);
            }
            else if(reason=="FullNumberError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("The number of collaborating users has been full about this swc!"),
                                         QMessageBox::Ok);
            }
        }

    }
}

void V3dR_Communicator::processAnalyzeMsg(QString line){
    qDebug()<<line;
    QRegExp analyzereg("/FEEDBACK_ANALYZE_(.*):(.*)");
    QRegExp definereg("/FEEDBACK_DEFINE_(.*):(.*)");
    line=line.trimmed();
    if(analyzereg.indexIn(line) != -1)
    {
        QString type=analyzereg.cap(1).trimmed();
        QString operatorMsg=analyzereg.cap(2).trimmed();

        qDebug()<<"type:"<<type;
        qDebug()<<"operatormsg:"<<operatorMsg;
        if(type=="SomaNearBy"){
            QString sender=operatorMsg.split(" ").at(0).trimmed();
            QString request_senderid=operatorMsg.split(" ").at(1).trimmed();
            int result=operatorMsg.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma is not connected to one point!"),
                                         QMessageBox::Ok);
            }else if(sender=="server" && result==-1){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma not detected!"),
                                         QMessageBox::Ok);
            }
        }
        if(type=="ColorMutation"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                emit addManyMarkers(msgList.join(","), "Color mutation");

                emit updateQcInfo();
                emit updateQcMarkersCounts();

                if( request_senderid==userId )
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: color mutation exists! notice the soma nearby and the red markers!"),
                                             QMessageBox::Ok);
            }else if(sender=="server" && result==-1){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma not detected!"),
                                         QMessageBox::Ok);
            }
        }
        if(type=="Dissociative"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                emit addManyMarkers(msgList.join(","), "Isolated branch");

                emit updateQcInfo();
                emit updateQcMarkersCounts();

                if(request_senderid==userId)
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: Isolated branch exists! notice the red markers!"),
                                             QMessageBox::Ok);
            }
        }
        if(type=="Angle"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                emit addManyMarkers(msgList.join(","), "Angle error");

                emit updateQcInfo();
                emit updateQcMarkersCounts();

                if(request_senderid==userId)
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: incorrect angle exists! notice the red markers!"),
                                             QMessageBox::Ok);
            }else if(sender=="server" && result==-1){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma not detected!"),
                                         QMessageBox::Ok);
            }
        }
    }
    else if(definereg.indexIn(line) != -1){
        QString type=definereg.cap(1).trimmed();
        QString operatorMsg=definereg.cap(2).trimmed();

        qDebug()<<"type:"<<type;
        qDebug()<<"operatormsg:"<<operatorMsg;
        if(type=="Soma"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();

            if (sender=="server" && result==0)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         "error: " + msgList.join(","),
                                         QMessageBox::Ok);
            }
            if(sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         msgList.join(","),
                                         QMessageBox::Ok);
            }
        }
    }
}

void V3dR_Communicator::processSendMsg(QString line){
    QRegExp sendreg("/FEEDBACK_SEND_(.*):(.*)");
    line=line.trimmed();
    if(sendreg.indexIn(line)!=-1)
    {
        QString type=sendreg.cap(1).trimmed();
        QString operatorMsg=sendreg.cap(2).trimmed();
        QString msg = operatorMsg;

        QStringList listWithHeader=msg.split(',',QString::SkipEmptyParts);
        QString msgHeader=listWithHeader[0];
        QString sender=msgHeader.split(" ").at(0).trimmed();
        QString request_senderid=msgHeader.split(" ").at(1).trimmed();
        int result=msgHeader.split(" ").at(2).trimmed().toInt();
        QStringList msgList=listWithHeader;
        msgList.removeAt(0);

        if (sender=="server" && type=="SomaPos")
        {
            if (result==0)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         "error: " + msgList[0],
                                         QMessageBox::Ok);
            }
            if(result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         msgList[0],
                                         QMessageBox::Ok);
            }
        }

    }
}

void V3dR_Communicator::TFProcess(QString line) {
    QRegExp msgreg("/(.*)_(.*):(.*)");

    line=line.trimmed();
    if(msgreg.indexIn(line)!=-1)
    {
        QString operationtype=msgreg.cap(1).trimmed();
        bool isNorm=msgreg.cap(2).trimmed()=="norm";
        QString operatorMsg=msgreg.cap(3).trimmed();

        if(operationtype == "drawline" )
        {
            QString msg=operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"drawline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            int isBegin=listwithheader[0].split(" ").at(2).toUInt();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit addSeg(listwithheader.join(","), isBegin);
            }
        }else if(operationtype == "delline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"delline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QStringList infos=listwithheader[0].split(" ");
            bool isTeraFly=infos.at(0).trimmed()=="0";
            QString user=infos.at(1).trimmed();
            unsigned int isMany=0;
            if(infos.size()>=6)
                isMany=infos.at(5).trimmed().toInt();

            if(user == "server"){
                QString qcInfo = infos.at(2).trimmed();
                int num = infos.at(3).trimmed().toInt();
                if(qcInfo == "overlap"){
                    removedOverlapSegNum += num;
                }
                else if(qcInfo == "error"){
                    removedErrSegNum += num;
                }
                emit updateQcSegsCounts();
            }

            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit delSeg(listwithheader.join(","),isMany);
            }
        }else if(operationtype=="splitline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"splitline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QStringList infos=listwithheader[0].split(" ");
            bool isTeraFly=infos.at(0).trimmed()=="0";
            QString user=infos.at(1).trimmed();

            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==useriD" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit splitSeg(listwithheader.join(","));
            }
        }else if(operationtype == "addmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"addmarker_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                emit addMarker(listwithheader[1], "");
            }
        }else if(operationtype == "delmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"delmarker_msg_count"<<listwithheader.count();
            if(listwithheader.size()<=1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit delMarker(listwithheader.join(","));
            }
        }else if(operationtype == "retypemarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"retypemarker_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                emit retypeMarker(listwithheader[1]);
            }
        }
        else if(operationtype == "retypeline")
        {
            QString msg =operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"retypeline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<=1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }

            QStringList infos=listwithheader[0].split(" ");

            bool isTeraFly=infos.at(0).trimmed()=="0";
            QString user=infos.at(1).trimmed();
            int type=infos.at(2).trimmed().toInt();
            unsigned int isMany=0;
            if(infos.size()>=7)
                isMany=infos.at(6).trimmed().toInt();
            //qDebug()<<"type = listwithheader[0]"<<listwithheader;
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit retypeSeg(listwithheader.join(","),type,isMany);
            }
        }
        else if(operationtype == "connectline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader = msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"connectline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<=1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit connectSeg(listwithheader.join(","));
            }
        }
        else if(operationtype == "drawmanylines"){
            QString msg=operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"drawmanylines_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userId && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userId" << userId;
            else
            {
                listwithheader.removeAt(0);
                emit addManySegs(listwithheader.join(","));
            }
        }
    }
}

void V3dR_Communicator::UpdateAddSegMsg(V_NeuronSWC seg, vector<V_NeuronSWC> connectedSegs, QString clienttype, bool isBegin)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(int(isBegin)).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        result+="$";
        for(auto connectedseg:connectedSegs){
            result+=V_NeuronSWCToSendMSG(connectedseg);
            result+="$";
        }
        qDebug()<<"drawline_sendMsg"<<result.count();
        sendMsg(QString("/drawline_norm:"+result.join(",")));

        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        undoDeque.push_back(QString("/delline_undo:"+result.join(",")));
        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateAddManySegsMsg(vector<V_NeuronSWC> segs, QString clienttype){
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        for(int i=0;i<segs.size();i++){
            bool flag = true;
//            for(int j=0;j<segs[i].row.size();j++){
//                if(segs[i].row[j].x >= ImageMaxRes.x ||
//                    segs[i].row[j].y >= ImageMaxRes.y ||
//                    segs[i].row[j].z >= ImageMaxRes.z){
//                    flag = false;
//                    break;
//                }
//            }
//            if(!flag){
//                continue;
//            }
            if(segs[i].on){
//                QStringList coorResult;
//                for(int j=0;j<segs[i].row.size();j++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
//                {
//                    V_NeuronSWC_unit curSWCunit = segs[i].row[j];
//                    XYZ GlobalCroods = ConvertLocalBlocktoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z);
//                    result.push_back(QString("%1 %2 %3 %4").arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z));
//                    //        if(i==seg.row.size()-1)
//                    //            AutoTraceNode=XYZ(GlobalCroods.x,GlobalCroods.y,GlobalCroods.z);
//                }
                result+=V_NeuronSWCToSendMSG(segs[i]);
                result.push_back("$");
            }
        }
        qDebug()<<"drawmanylines_sendMsg"<<result.count();
        sendMsg(QString("/drawmanylines_norm:"+result.join(",")));
    }
}

void V3dR_Communicator::UpdateConnectSegMsg(XYZ p1, XYZ p2, V_NeuronSWC seg1, V_NeuronSWC seg2, QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        XYZ GlobalCrood1 = ConvertLocalBlocktoGlobalCroods(p1.x,p1.y,p1.z);
        XYZ GlobalCrood2 = ConvertLocalBlocktoGlobalCroods(p2.x,p2.y,p2.z);
        result.push_back(QString("%1 %2 %3").arg(GlobalCrood1.x).arg(GlobalCrood1.y).arg(GlobalCrood1.z));
        result.push_back(QString("%1 %2 %3").arg(GlobalCrood2.x).arg(GlobalCrood2.y).arg(GlobalCrood2.z));
        result+=V_NeuronSWCToSendMSG(seg1);
        result.push_back("$");
        result+=V_NeuronSWCToSendMSG(seg2);
        result.push_back("$");
        qDebug()<<"connectline_sendMsg"<<result.count();
        //        qDebug()<<"/connectline_norm:"+result.join(",");
        sendMsg(QString("/connectline_norm:"+result.join(",")));
        //        result+=V_NeuronSWCToSendMSG(seg);
        //        qDebug()<<"connectline_sendMsg"<<result.count();
        //        sendMsg(QString("/connectline_norm:"+result.join(",")));
        //        while(undoDeque.size()>=dequeszie)
        //        {
        //            undoDeque.pop_front();
        //        }
        //        undoDeque.push_back(QString("/delline_undo:"+result.join(",")));
        //        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype,vector<V_NeuronSWC> connectedSegs, bool isBegin)
{
    if(clienttype=="TeraFly")
    {
        if(seg.row.size() == 0){
            return;
        }
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        qDebug()<<"delline_sendMsg"<<result.count();
        sendMsg(QString("/delline_norm:"+result.join(",")));

        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        QString undoMsgHeader=QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(int(isBegin)).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);
        QStringList undoMsg;
        undoMsg.push_back(undoMsgHeader);
        undoMsg+=V_NeuronSWCToSendMSG(seg);
        undoMsg+="$";
        for(auto connectedseg:connectedSegs){
            undoMsg+=V_NeuronSWCToSendMSG(connectedseg);
            undoMsg+="$";
        }
        undoDeque.push_back(QString("/drawline_undo:"+undoMsg.join(",")));
        redoDeque.clear();

    }
}

void V3dR_Communicator::UpdateDelManySegMsg(vector<V_NeuronSWC> segs, QString clienttype, vector<vector<V_NeuronSWC>> connectedSegs, vector<bool> isBeginVec){
    if(segs.size() == 0){
        return;
    }

    QStringList result;
    result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z).arg(1));
    for(int i = 0; i < segs.size(); i++){
        if(clienttype=="TeraFly")
        {
            if(segs[i].row.size() == 0){
                continue;
            }
            result+=V_NeuronSWCToSendMSG(segs[i]);
            result+="$";

            while(undoDeque.size()>=dequeszie)
            {
                undoDeque.pop_front();
            }
            QString undoMsgHeader=QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(int(isBeginVec[i])).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);
            QStringList undoMsg;
            undoMsg.push_back(undoMsgHeader);
            undoMsg+=V_NeuronSWCToSendMSG(segs[i]);
            undoMsg+="$";
            for(auto connectedseg:connectedSegs[i]){
                undoMsg+=V_NeuronSWCToSendMSG(connectedseg);
                undoMsg+="$";
            }
            undoDeque.push_back(QString("/drawline_undo:"+undoMsg.join(",")));
            redoDeque.clear();
        }
    }
    if(clienttype=="TeraFly"){
        sendMsg(QString("/delline_norm:"+result.join(",")));
    }
}

void V3dR_Communicator::UpdateDelManyConnectedSegsMsg(vector<V_NeuronSWC> segs,QString clienttype){
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z).arg(1));
        for(int i=0;i<segs.size();i++){
            if(segs[i].on){
                result+=V_NeuronSWCToSendMSG(segs[i]);
                result.push_back("$");
            }
        }

        qDebug()<<"delline_sendMsg"<<result.count()<<"!";
        sendMsg(QString("/delline_norm:"+result.join(",")));
        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }

        QStringList undo_result;
        undo_result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z).arg(1));
        for(int i=0;i<segs.size();i++){
            if(segs[i].on){
                reverse(segs[i].row.begin(), segs[i].row.end());
                undo_result+=V_NeuronSWCToSendMSG(segs[i]);
                undo_result.push_back("$");
            }
        }

        undoDeque.push_back(QString("/drawmanylines_undo:"+undo_result.join(",")));
        redoDeque.clear();

    }
}

void V3dR_Communicator::UpdateAddMarkerMsg(float X, float Y, float Z,RGBA8 color,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        XYZ global_node=ConvertLocalBlocktoGlobalCroods(X,Y,Z);
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/addmarker_norm:"+result.join(",")));

        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        QString undoMsgHeader=QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);
        QStringList undoMsg;
        undoMsg.push_back(undoMsgHeader);
        undoMsg.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        undoDeque.push_back(QString("/delmarker_undo:"+undoMsg.join(",")));
        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateRetypeMarkerMsg(float X,float Y,float Z, RGBA8 color, QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        XYZ global_node=ConvertLocalBlocktoGlobalCroods(X,Y,Z);
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/retypemarker_norm:"+result.join(",")));

    }
}

void V3dR_Communicator::UpdateDelMarkersMsg(vector<CellAPO> markers,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        for(auto it=markers.begin(); it!=markers.end(); it++){
            XYZ global_node=ConvertLocalBlocktoGlobalCroods(it->x,it->y,it->z);
            RGBA8 color = it->color;
            result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));

            while(undoDeque.size()>=dequeszie)
            {
                undoDeque.pop_front();
            }
            QString undoMsgHeader=QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);
            QStringList undoMsg;
            undoMsg.push_back(undoMsgHeader);
            undoMsg.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));
            undoDeque.push_back(QString("/addmarker_undo:"+undoMsg.join(",")));
        }
        sendMsg(QString("/delmarker_norm:"+result.join(",")));

        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        if(seg.row.size() == 0){
            return;
        }
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userId).arg(type).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        qDebug()<<"retypeline_sendMsg"<<result.count();
        sendMsg(QString("/retypeline_norm:"+result.join(",")));
        qDebug()<<"retypeline_norm"+result.join(",");
    }
}

void V3dR_Communicator::UpdateRetypeManySegsMsg(vector<V_NeuronSWC> segs,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6 %7").arg(0).arg(userId).arg(type).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z).arg(1));

        for(int i=0;i<segs.size();i++){
            if(segs[i].row.size() == 0){
                continue;
            }
            result+=V_NeuronSWCToSendMSG(segs[i]);
            result.push_back("$");
        }

        qDebug()<<"retypeline_sendMsg"<<result.count()<<"!!!!!!!!";
        sendMsg(QString("/retypeline_norm:"+result.join(",")));
    }
}

void V3dR_Communicator::UpdateAddSegMsg(QString TVaddSegMSG)
{
    this->sendMsg("/drawline_norm:"+TVaddSegMSG);
    while(undoDeque.size()>=dequeszie)
    {
        undoDeque.pop_front();
    }
    undoDeque.push_back(QString("/delline_undo:"+TVaddSegMSG));
    redoDeque.clear();
}

void V3dR_Communicator::UpdateDelSegMsg(QString TVdelSegMSG)
{
    this->sendMsg("/delline_norm:"+TVdelSegMSG);
    while(undoDeque.size()>=dequeszie)
    {
        undoDeque.pop_front();
    }
    undoDeque.push_back(QString("/drawline_undo:"+TVdelSegMSG+",$"));
    redoDeque.clear();
}

void V3dR_Communicator::UpdateAddMarkerMsg(QString TVaddMarkerMSG)
{
    this->sendMsg("/addmarker_norm:"+TVaddMarkerMSG);

}

void V3dR_Communicator::UpdateDelMarkerMsg(QString TVdelMarkerMSG)
{
    this->sendMsg("/delmarker_norm:"+TVdelMarkerMSG);

}

void V3dR_Communicator::UpdateRetypeSegMsg(QString TVretypeSegMSG)
{
    this->sendMsg("/retypeline_norm:"+TVretypeSegMSG);
}

void V3dR_Communicator::UpdateSplitSegMsg(V_NeuronSWC seg,V3DLONG nodeinseg_id,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        V_NeuronSWC_list new_slist = split_V_NeuronSWC_simplepath (seg, nodeinseg_id);
        if(new_slist.seg.size()==1)
            return;
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userId).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        result.push_back("$");

        for(auto newseg:new_slist.seg){
            result+=V_NeuronSWCToSendMSG(newseg);
            result.push_back("$");
        }

        qDebug()<<"splitline_sendMsg"<<result.count();
        sendMsg(QString("/splitline_norm:"+result.join(",")));

    }
    //    V_NeuronSWC_list new_slist = split_V_NeuronSWC_simplepath (seg, nodeinseg_id);
    //    UpdateDelSegMsg(seg,clienttype);

    //    for(auto newseg:new_slist.seg)
    //    {
    //        UpdateAddSegMsg(newseg,clienttype);
    //    }
}

void V3dR_Communicator::UpdateSplitSegMsg(QString deleteMsg,QString addMsg1,QString addMsg2)
{
    UpdateDelSegMsg(deleteMsg);
    UpdateAddSegMsg(addMsg1);
    UpdateAddSegMsg(addMsg2);
}

void V3dR_Communicator::UpdateUndoDeque()
{
    if(undoDeque.size()!=0)
    {
        QString msg=undoDeque.back();
        QRegExp reg("/(.*)_(.*):(.*)");
        if(reg.indexIn(msg)!=-1)
        {
            QString operationType=reg.cap(1);
            QString operatorMsg=reg.cap(3);

            if("delline"==operationType){
                QString newMsg="";
                QStringList pointlist=operatorMsg.split(",",QString::SkipEmptyParts);
                QString msgHeader=pointlist[0];
                QStringList headerList=msgHeader.split(" ", QString::SkipEmptyParts);
                headerList.removeAt(2);

                QString newMsgHeader=headerList.join(" ");
                newMsg=newMsgHeader;
                newMsg+=",";
                for(int i=1;i<pointlist.size();i++){
                    if(pointlist[i]=="$"){
                        break;
                    }else{
                        newMsg+=pointlist[i];
                        newMsg+=",";
                    }
                }
                newMsg.chop(1);
                msg="/"+operationType+"_undo:"+newMsg;
            }

            sendMsg(msg);
            undoDeque.pop_back();

            if("drawline"==operationType)
                operationType="/delline";
            else if("delline"==operationType)
                operationType="/drawline";
            else if("addmarker"==operationType)
                operationType="/delmarker";
            else if("delmarker"==operationType)
                operationType="/addmarker";

            while(redoDeque.size()>=dequeszie)
            {
                redoDeque.pop_front();
            }
            if("drawmanylines"!=operationType)
                redoDeque.push_back(QString(operationType+"_redo:"+operatorMsg));
        }
    }else
    {
        QMessageBox::information(0,tr("Warning "),
                                 tr("Action can not be implemented"),
                                 QMessageBox::Ok);
    }
}

void V3dR_Communicator::UpdateRedoDeque()
{
    if(redoDeque.size()!=0)
    {
        QString msg=redoDeque.back();
        QRegExp reg("/(.*)_(.*):(.*)");
        if(reg.indexIn(msg)!=-1)
        {
            QString operationType=reg.cap(1);
            QString operatorMsg=reg.cap(3);

            if("delline"==operationType){
                QString newMsg="";
                QStringList pointlist=operatorMsg.split(",",QString::SkipEmptyParts);
                QString msgHeader=pointlist[0];
                QStringList headerList=msgHeader.split(" ", QString::SkipEmptyParts);
                headerList.removeAt(2);

                QString newMsgHeader=headerList.join(" ");
                newMsg=newMsgHeader;
                newMsg+=",";
                for(int i=1;i<pointlist.size();i++){
                    if(pointlist[i]=="$"){
                        break;
                    }else{
                        newMsg+=pointlist[i];
                        newMsg+=",";
                    }
                }
                newMsg.chop(1);
                msg="/"+operationType+"_redo:"+newMsg;
            }

            //            qDebug()<<msg;
            sendMsg(msg);
            redoDeque.pop_back();

            if("drawline"==operationType)
                operationType="/delline";
            else if("delline"==operationType)
                operationType="/drawline";
            else if("addmarker"==operationType)
                operationType="/delmarker";
            else if("delmarker"==operationType)
                operationType="/addmarker";

            while(undoDeque.size()>=dequeszie)
            {
                undoDeque.pop_front();
            }
            undoDeque.push_back(QString(operationType+"_undo:"+operatorMsg));
        }
    }else
    {
        QMessageBox::information(0,tr("Warning "),
                                 tr("Action can not be implemented"),
                                 QMessageBox::Ok);
    }
}

void V3dR_Communicator::setAddressIP(QString addressIp){
    m_strAddressIP=addressIp;
}

void V3dR_Communicator::setPort(uint port){
    m_iPort=port;
}

void V3dR_Communicator::onConnected() {
    qDebug()<<userId;
    qDebug()<<"Message onConnected";
    b_isConnectedState=true;
    reconnectCnt=0;
    QString RES=QString("RES(%1x%2x%3)").arg(ImageMaxRes.y).arg(ImageMaxRes.x).arg(ImageMaxRes.z);

    sendMsg(QString("/login:%1 %2 %3 %4 %5 %6").arg(userId).arg(userName).arg(password).arg(RES).arg(LoadManageWidget::curSwcUuid).arg(0));
//    sendMsg(QString("/login:%1 %2 %3").arg(userId).arg(RES).arg(0));
    QString msg = "Connect success! Ready to start collaborating";
    QMessageBox messageBox;
    messageBox.setWindowTitle(tr("Information"));
    messageBox.setText(tr(msg.toStdString().c_str()));
    messageBox.setIcon(QMessageBox::Information);
    QTimer::singleShot(800, &messageBox, SLOT(accept()));
    messageBox.exec();

//    QMessageBox::information(0,tr("Message "),
//                             tr("Connect success! Ready to start collaborating"),
//                             QMessageBox::Ok);

}

//void V3dR_Communicator::initConnect(){
//    qDebug()<<"enter initConnect";
//    if(b_isConnectedState)
//    {
//        timer_iniconn->stop();
//        return;
//    }

//    if(socket->state() != QAbstractSocket::UnconnectedState)
//        return;
//    // 未连接时，每隔五秒自动向指定服务器发送连接请求
//    if(socket->state() == QAbstractSocket::UnconnectedState)
//    {
//        initConnectCnt++;
//        if(initConnectCnt>=6){
//            QMessageBox::information(0,tr("Message "),
//                                     tr("Connect failed. Please Restart Vaa3d"),
//                                     QMessageBox::Ok);

//            qDebug() <<"Connect failed."<<socket->errorString();
//            emit exit();
//            timer_iniconn->stop();
//        }
//        qDebug()<<"initconnect state:"<<socket->state();
//        socket->abort();
//        socket->connectToHost(m_strAddressIP, m_iPort);
//    }

//    // 连接成功后关闭初始化连接定时器，后面的断线重连与此无关
//    else if(socket->state() == QAbstractSocket::ConnectedState)
//    {
//        timer_iniconn->stop();
//    }

//}

//void V3dR_Communicator::autoReconnect(){
//    if(b_isConnectedState)
//        return;

//    if(socket->state() != QAbstractSocket::UnconnectedState)
//        return;

//    reconnectCnt++;
//    if(reconnectCnt>=6){
//        QMessageBox::information(0,tr("Message "),
//                                 tr("Reconnect failed. Please Restart Vaa3d"),
//                                 QMessageBox::Ok);

//        qDebug() <<"Reconnect failed."<<socket->errorString();
//        emit exit();
//        m_timerConnect->stop();
//    }
//    qDebug()<<"reconnect state:"<<socket->state();
//    socket->abort();
//    socket->connectToHost(m_strAddressIP, m_iPort);
////    if(!socket->waitForConnected(1000*5))
////    {
////        QMessageBox::information(0,tr("Message "),
////                                 tr(QString("Reconnect failed %1").arg(reconnectCnt).toStdString().c_str()),
////                                 QMessageBox::Ok);

////        qDebug() <<"reconnect failed!"<<socket->errorString();
////        if(reconnectCnt>=5){
////            QMessageBox::information(0,tr("Message "),
////                                     tr("Too many reconnection failures. Please Restart Vaa3d"),
////                                     QMessageBox::Ok);

////            qDebug() <<"Too many reconnection failures!"<<socket->errorString();
////            emit exit();
////            m_timerConnect->stop();
////        }
////    }

//}

void V3dR_Communicator::autoExit(){
    qDebug()<<"enter autoExit";
    qDebug()<<this->socket;
    if(!this->socket)
        return;
    this->socket->disconnectFromHost();
    QMessageBox::information(0,tr("Message "),
                             tr("Long time no operation, please try to reload the anofile!"),
                             QMessageBox::Ok);
//    timer_exit->stop();
}

void V3dR_Communicator::resetWarnMulBifurcationFlag(){
    b_isWarnMulBifurcationHandled=false;
}

void V3dR_Communicator::resetWarnLoopFlag(){
    b_isWarnLoopHandled=false;
}

QStringList V3dR_Communicator::V_NeuronSWCToSendMSG(V_NeuronSWC seg)
{
    QStringList result;
    for(int i=0;i<seg.row.size();i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
    {
        V_NeuronSWC_unit curSWCunit = seg.row[i];
        XYZ GlobalCroods = ConvertLocalBlocktoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z);
        result.push_back(QString("%1 %2 %3 %4").arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z));
        //        if(i==seg.row.size()-1)
        //            AutoTraceNode=XYZ(GlobalCroods.x,GlobalCroods.y,GlobalCroods.z);
    }
    return result;
}

XYZ V3dR_Communicator::ConvertGlobaltoLocalBlockCroods(double x,double y,double z)
{
    auto node=ConvertMaxRes2CurrResCoords(x,y,z);
    node.x-=(ImageStartPoint.x-1);
    node.y-=(ImageStartPoint.y-1);
    node.z-=(ImageStartPoint.z-1);
    return node;
}

XYZ V3dR_Communicator::ConvertLocalBlocktoGlobalCroods(double x,double y,double z)
{
    x+=(ImageStartPoint.x-1);
    y+=(ImageStartPoint.y-1);
    z+=(ImageStartPoint.z-1);

    XYZ node=ConvertCurrRes2MaxResCoords(x,y,z);
    return node;
}

XYZ V3dR_Communicator::ConvertMaxRes2CurrResCoords(double x,double y,double z)
{
    x/=(ImageMaxRes.x/ImageCurRes.x);
    y/=(ImageMaxRes.y/ImageCurRes.y);
    z/=(ImageMaxRes.z/ImageCurRes.z);
    return XYZ(x,y,z);
}

XYZ V3dR_Communicator::ConvertCurrRes2MaxResCoords(double x,double y,double z)
{
    //    qDebug()<<ImageMaxRes.x/ImageCurRes.x;
    x*=(ImageMaxRes.x/ImageCurRes.x);
    y*=(ImageMaxRes.y/ImageCurRes.y);
    z*=(ImageMaxRes.z/ImageCurRes.z);
    return XYZ(x,y,z);
}

void V3dR_Communicator::resetdatatype()
{
    datatype.isFile=false;
    datatype.datasize=0;
    datatype.filesize=0;
}

void V3dR_Communicator::checkConnectionForVR(){
    if(!this->socket || this->socket->state() != QAbstractSocket::ConnectedState){
        QString msg = "Disconnection! Start Reconnecting...";
        qDebug() << msg;
        terafly::CViewer *cur_win = terafly::CViewer::getCurrent();
//        while(cur_win && cur_win->getGLWidget() && cur_win->getGLWidget()->myvrwin && !cur_win->getGLWidget()->myvrwin->isQuit){
//            cur_win->getGLWidget()->myvrwin->isQuit = true;
//        }
        if(cur_win && cur_win->getGLWidget() && cur_win->getGLWidget()->myvrwin){
            cur_win->getGLWidget()->myvrwin->isQuit = true;
        }
        emit resetConn(LoadManageWidget::m_port);

            //                if(cur_win->view3DWidget->myvrwin->pMainApplication){
            //                    cur_win->view3DWidget->myvrwin->shutdown();
            //                    delete cur_win->view3DWidget->myvrwin;
            //                    cur_win->view3DWidget->myvrwin=nullptr;
            //                }
            //            }
            //            doCollaborationVRView();
    }
}
