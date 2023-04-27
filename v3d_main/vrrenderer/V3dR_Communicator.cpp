#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
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
QString V3dR_Communicator::userName="";
V3dR_Communicator::V3dR_Communicator(QObject *partent):b_isConnectedState(false), b_isWarnMulBifurcationHandled(false), b_isWarnLoopHandled(false), reconnectCnt(0), initConnectCnt(0), QObject(partent)
{
	CreatorMarkerPos = 0;
	CreatorMarkerRes = 0;
	userName="";
    qDebug()<<"userName=\"\"";

//    socket = new QTcpSocket(this);
    resetdatatype();
    connect(this,SIGNAL(msgtoprocess(QString)),this,SLOT(TFProcess(QString)));
    connect(this,SIGNAL(msgtowarn(QString)),this,SLOT(processWarnMsg(QString)));
//    connect(this->socket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
    m_timerConnect = new QTimer(this);
    timer_iniconn=new QTimer(this);
    timer_exit=new QTimer(this);
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
                        qDebug() << userName << " receive not match format\n";
                        socket->disconnectFromHost();
                    }
                    qDebug()<<msg;
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
                    std::cout<<QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ").toStdString()<<" receive from "<<userName.toStdString()<<" :"<<data<<std::endl;
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
    qDebug()<<"send:"<<msg;
    const std::string data=msg.toStdString();
    const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
    socket->write(header.c_str(),header.size());
    socket->write(data.c_str(),data.size());
    socket->flush();
    if(timer_exit->isActive()){
        timer_exit->stop();
    }
    timer_exit->start(2*60*60*1000);
}

void V3dR_Communicator::preprocessmsgs(QStringList list)
{
    //1. 开始协作
    //2. 更新用户
    //3. 处理协作指令
    qDebug()<<"begin to preprocessmsgs";

    QRegExp usersRex("^/activeusers:(.*)$");
    QRegExp warnRex("^/WARN_(.*)$");
    for(auto &msg:list)
    {
        qDebug()<<"OnRead:"<<msg;
        if(msg.startsWith("STARTCOLLABORATE:")){
//            qDebug()<<"start collaborate_msg____Debug_zll"<<msg;
            emit load(msg.right(msg.size()-QString("STARTCOLLABORATE:").size()));
        }else if(usersRex.indexIn(msg) != -1){
            emit updateuserview(usersRex.cap(1));
        }else if(warnRex.indexIn(msg) != -1){
            emit msgtowarn(msg);
        }
        else{
            emit msgtoprocess(msg);
        }
    }
}

void V3dR_Communicator::processWarnMsg(QString line,bool flag_init){
    QRegExp warnreg("/WARN_(.*):(.*)");
    line=line.trimmed();
    if(warnreg.indexIn(line)!=-1)
    {
        QString reason=warnreg.cap(1).trimmed();
        QString operatorMsg=warnreg.cap(2).trimmed();

//        if(reason=="MulBifurcation"&&!b_isWarnMulBifurcationHandled)
//        {
//            QMessageBox::warning(0,tr("Warning "),
//                                     tr("Multiple bifurcation exists! Notice the brown markers."),
//                                     QMessageBox::Ok);
//            b_isWarnMulBifurcationHandled=true;
//            QTimer::singleShot(30*1000,this,SLOT(resetWarnMulBifurcationFlag()));
//        }

//        if(reason=="Loop"&&!b_isWarnLoopHandled)
//        {
//            QMessageBox::warning(0,tr("Warning "),
//                                     tr("Loop exists! Notice the black markers."),
//                                     QMessageBox::Ok);
//            b_isWarnLoopHandled=true;
//            QTimer::singleShot(30*1000,this,SLOT(resetWarnLoopFlag()));
//        }

        QString msg = operatorMsg;
        QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
        if(listwithheader.size()<1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }
        bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
        QString sender=listwithheader[0].split(" ").at(1).trimmed();
        if (sender=="server" && isTeraFly)
        {
            qDebug() << "sender:" << sender;
            emit addMarker(listwithheader[1]);
        }

    }
}

void V3dR_Communicator::TFProcess(QString line,bool flag_init) {
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
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit addSeg(listwithheader.join(","));
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
            qDebug() << "+============delseg process begin========";
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit delSeg(listwithheader.join(","));
            }
        }else if(operationtype == "addmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                emit addMarker(listwithheader[1]);
            }
        }else if(operationtype == "delmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                emit delMarker(listwithheader[1]);
            }
        }else if(operationtype == "retypemarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
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

            bool isTeraFly=listwithheader[0].split(" ").at(0).trimmed()=="0";
            QString user=listwithheader[0].split(" ").at(1).trimmed();
            int type=listwithheader[0].split(" ").at(2).trimmed().toInt();
            qDebug()<<"type = listwithheader[0]"<<listwithheader;
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit retypeSeg(listwithheader.join(","),type);
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
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit connectSeg(listwithheader.join(","));
            }

        }
    }
}

void V3dR_Communicator::UpdateAddSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
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

void V3dR_Communicator::UpdateConnectSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        qDebug()<<"connectline_sendMsg"<<result.count();
        sendMsg(QString("/connectline_norm:"+result.join(",")));
        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        undoDeque.push_back(QString("/delline_undo:"+result.join(",")));
        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        qDebug()<<"delline_sendMsg"<<result.count();
        sendMsg(QString("/delline_norm:"+result.join(",")));
        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        undoDeque.push_back(QString("/drawline_undo:"+result.join(",")));
        redoDeque.clear();

    }
}

void V3dR_Communicator::UpdateAddMarkerMsg(float X, float Y, float Z,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        XYZ global_node=ConvertLocalBlocktoGlobalCroods(X,Y,Z);
        result.push_back(QString("%1 %2 %3 %4").arg(type).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/addmarker_norm:"+result.join(",")));

    }
}

void V3dR_Communicator::UpdateRetypeMarkerMsg(float X,float Y,float Z, RGBA8 color, QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        XYZ global_node=ConvertLocalBlocktoGlobalCroods(X,Y,Z);
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(color.r).arg(color.g).arg(color.b).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/retypemarker_norm:"+result.join(",")));

    }
}

void V3dR_Communicator::UpdateDelMarkerSeg(float x,float y,float z,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(0).arg(userName).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        XYZ global_node=ConvertLocalBlocktoGlobalCroods(x,y,z);
        result.push_back(QString("%1 %2 %3 %4").arg(-1).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/delmarker_norm:"+result.join(",")));

    }
}

void V3dR_Communicator::UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(0).arg(userName).arg(type).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        qDebug()<<"retypeline_sendMsg"<<result.count();
        sendMsg(QString("/retypeline_norm:"+result.join(",")));
        qDebug()<<"retypeline_norm"+result.join(",");
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
    undoDeque.push_back(QString("/drawline_undo:"+TVdelSegMSG));
    redoDeque.clear();
}

void V3dR_Communicator::UpdateAddMarkerMsg(QString TVaddMarkerMSG)
{
    this->sendMsg("/addmarker_norm:"+TVaddMarkerMSG);

}

void V3dR_Communicator::UpdateDelMarkerSeg(QString TVdelMarkerMSG)
{
    this->sendMsg("/delmarker_norm:"+TVdelMarkerMSG);

}

void V3dR_Communicator::UpdateRetypeSegMsg(QString TVretypeSegMSG)
{
    this->sendMsg("/retypeline_norm:"+TVretypeSegMSG);
}

void V3dR_Communicator::UpdateSplitSegMsg(V_NeuronSWC seg,V3DLONG nodeinseg_id,QString clienttype)
{
    UpdateDelSegMsg(seg,clienttype);
    V_NeuronSWC_list new_slist = split_V_NeuronSWC_simplepath (seg, nodeinseg_id);
    for(auto newseg:new_slist.seg)
    {
        UpdateAddSegMsg(newseg,clienttype);
    }
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
            sendMsg(msg);
            undoDeque.pop_back();

            QString operationType=reg.cap(1);
            QString operatorMsg=reg.cap(3);
            if("drawline"==operationType)
                operationType="/delline";
            else if("delline"==operationType)
                operationType="/drawline";
            while(redoDeque.size()>=dequeszie)
            {
                redoDeque.pop_front();
            }
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
            sendMsg(msg);
            redoDeque.pop_back();

            QString operationType=reg.cap(1);
            QString operatorMsg=reg.cap(3);
            if("drawline"==operationType)
                operationType="/delline";
            else if("delline"==operationType)
                operationType="/drawline";
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
    qDebug()<<userName;
    qDebug()<<"Message onConnected";
    b_isConnectedState=true;
    m_timerConnect->stop();
    reconnectCnt=0;
    //发送用户ID和设备类型 0 桌面设备，可查看全脑，需要发送全脑图像
    sendMsg(QString("/login:%1 %2").arg(userName).arg(0));
    QMessageBox::information(0,tr("Message "),
                             tr("Connect success! Ready to start collaborating"),
                             QMessageBox::Ok);
//    if(timer_exit->isActive())
//        timer_exit->stop();
//    timer_exit->start(30*1000);
}

void V3dR_Communicator::initConnect(){
    qDebug()<<"enter initConnect";
    if(b_isConnectedState)
    {
        timer_iniconn->stop();
        return;
    }

    if(socket->state() != QAbstractSocket::UnconnectedState)
        return;
    // 未连接时，每隔五秒自动向指定服务器发送连接请求
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        initConnectCnt++;
        if(initConnectCnt>=6){
            QMessageBox::information(0,tr("Message "),
                                     tr("Connect failed. Please Restart Vaa3d"),
                                     QMessageBox::Ok);

            qDebug() <<"Connect failed."<<socket->errorString();
            emit exit();
            timer_iniconn->stop();
        }
        qDebug()<<"initconnect state:"<<socket->state();
        socket->abort();
        socket->connectToHost(m_strAddressIP, m_iPort);
    }

    // 连接成功后关闭初始化连接定时器，后面的断线重连与此无关
    else if(socket->state() == QAbstractSocket::ConnectedState)
    {
        timer_iniconn->stop();
    }

}

void V3dR_Communicator::autoReconnect(){
    if(b_isConnectedState)
        return;

    if(socket->state() != QAbstractSocket::UnconnectedState)
        return;

    reconnectCnt++;
    if(reconnectCnt>=6){
        QMessageBox::information(0,tr("Message "),
                                 tr("Reconnect failed. Please Restart Vaa3d"),
                                 QMessageBox::Ok);

        qDebug() <<"Reconnect failed."<<socket->errorString();
        emit exit();
        m_timerConnect->stop();
    }
    qDebug()<<"reconnect state:"<<socket->state();
    socket->abort();
    socket->connectToHost(m_strAddressIP, m_iPort);
//    if(!socket->waitForConnected(1000*5))
//    {
//        QMessageBox::information(0,tr("Message "),
//                                 tr(QString("Reconnect failed %1").arg(reconnectCnt).toStdString().c_str()),
//                                 QMessageBox::Ok);

//        qDebug() <<"reconnect failed!"<<socket->errorString();
//        if(reconnectCnt>=5){
//            QMessageBox::information(0,tr("Message "),
//                                     tr("Too many reconnection failures. Please Restart Vaa3d"),
//                                     QMessageBox::Ok);

//            qDebug() <<"Too many reconnection failures!"<<socket->errorString();
//            emit exit();
//            m_timerConnect->stop();
//        }
//    }

}

void V3dR_Communicator::autoExit(){
    qDebug()<<"enter autoExit";
    qDebug()<<this->socket;
    if(!this->socket)
        return;
    this->socket->disconnectFromHost();
    QMessageBox::information(0,tr("Message "),
                             tr("Long time no operation, please try to reload the anofile!"),
                             QMessageBox::Ok);
    timer_exit->stop();
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
    qDebug()<<ImageMaxRes.x/ImageCurRes.x;
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
