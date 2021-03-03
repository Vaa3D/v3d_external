#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include <QRegExp>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>

V3dR_Communicator::V3dR_Communicator(QObject *partent):QObject(partent)
{
	CreatorMarkerPos = 0;
	CreatorMarkerRes = 0;
	userName="";

    socket = new QTcpSocket(this);
    resetDataType();
    connect(this,SIGNAL(msgtoprocess(QString)),this,SLOT(TFProcess(QString)));
    connect(this->socket,SIGNAL(connected()),this,SLOT(onConnected()));
//    connect(this->socket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));

}

void V3dR_Communicator::onReadyRead()
{    
    if(!datatype.isFile)
    {
        if(datatype.datasize==0)
        {
            if(socket->bytesAvailable()>=1024||socket->canReadLine())
            {
                QString msg=socket->readLine(1024);
                int ret=processHeader(msg);
                if(!ret) onReadyRead();
                else
                {
                    resetDataType();
                }
            }
        }else
        {
            int cnt=socket->bytesAvailable();
            if(cnt>=datatype.datasize)
            {
                QString msg=socket->readLine(datatype.datasize+1);
                resetDataType();
                emit msgtoprocess(msg);
                onReadyRead();
            }
        }
    }else if(socket->bytesAvailable())
    {
        int ret = 0;
        auto bytes=socket->read(datatype.datasize);
        if(datatype.f->write(bytes)==bytes.size())
        {
            datatype.datasize-=bytes.size();
            if(datatype.datasize==0)
            {
                QString filename=datatype.f->fileName();
                datatype.f->close();
                resetDataType();
//                processFile(filename);
                ret=0;
            }else if(datatype.datasize<0)
            {
                resetDataType();
                ret = 6;
            }
        }else{
            resetDataType();
            ret = 5;
        }
        if(!ret) onReadyRead();
    }
}

char V3dR_Communicator::processHeader(const QString rmsg)
{
    int ret = 0;
    if(rmsg.endsWith('\n'))
    {
        QString msg=rmsg.trimmed();
        if(msg.startsWith("DataTypeWithSize:"))
        {
            msg=msg.right(msg.size()-QString("DataTypeWithSize:").size());
            QStringList paras=msg.split(";;",QString::SkipEmptyParts);
            if(paras.size()==2&&paras[0]=="0")
            {
                datatype.datasize=paras[1].toInt();
            }else if(paras.size()==3&&paras[0]=="1")
            {
                datatype.isFile=true;
                datatype.filename=paras[1];
                datatype.datasize=paras[2].toInt();
                if(!QDir(QCoreApplication::applicationDirPath()+"/tmp").exists())
                    QDir(QCoreApplication::applicationDirPath()).mkdir("tmp");
                QString filePath=QCoreApplication::applicationDirPath()+"/tmp/"+datatype.filename;
                datatype.f=new QFile(filePath);
                if(!datatype.f->open(QIODevice::WriteOnly))
                    ret=4;
            }else
            {
                ret=3;
            }
        }else
        {
            ret = 2;
        }
    }else
    {
        ret = 1;
    }
    return ret;
}

void V3dR_Communicator::sendMsg(QString str)
{
    if(socket->state()==QAbstractSocket::ConnectedState)
    {
        const QString data=str+"\n";
        int datalength=data.size();
        QString header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(datalength);
        socket->write(header.toStdString().c_str(),header.size());
        socket->write(data.toStdString().c_str(),data.size());
        socket->flush();
    }
}

void V3dR_Communicator::processReaded(QStringList list)
{
    QStringList filepaths;
    for(auto msg:list)
    {
//        qDebug()<<msg;
        if(msg.startsWith("00"))
        {
            QRegExp usersRex("^/users:(.*)$");
            QString tmp=msg.remove(0,2);
            if(usersRex.indexIn(tmp) != -1)
            {
                emit updateuserview(usersRex.cap(1));
            }else
                emit msgtoprocess(tmp);
        }else if(msg.startsWith("11")&&msg.contains("swc"))
        {
            msg=msg.remove(0,2);
            emit load(msg.section("/",-1).section(".",0,1));
        }
    }
}

void V3dR_Communicator::resetDataType()
{
    datatype.isFile=false;
    datatype.datasize=0;
    datatype.filename.clear();
    if(datatype.f)
    {
        delete datatype.f;
        datatype.f=nullptr;
    }
}

void V3dR_Communicator::TFProcess(QString line,bool flag_init) {
//    QRegExp usersRex("^/users:(.*)$");
    QRegExp hmdposRex("^/hmdpos:(.*)$");
    QRegExp msgreg("^/(.*)_(.*):(.*)$");

    line=line.trimmed();
    if(msgreg.indexIn(line)!=-1)
    {
        QString operationtype=msgreg.cap(1).trimmed();
        bool isNorm=msgreg.cap(2).trimmed()=="norm";
        QString operatorMsg=msgreg.cap(3).trimmed();

        if(operationtype == "drawline" )
        {
            QString msg=operatorMsg;
            QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QString user=listwithheader[0].split(" ").at(0).trimmed();
            bool isTeraFly=listwithheader[0].split(" ").at(1).trimmed()=="TeraFly";
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit addSeg(listwithheader.join(";"));
            }
        }else if(operationtype == "delline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            qDebug() << "+============delseg process begin========";
            QString user=listwithheader[0].split(" ").at(0).trimmed();
            bool isTeraFly=listwithheader[0].split(" ").at(1).trimmed()=="TeraFly";
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit delSeg(listwithheader.join(";"));
            }
        }else if(operationtype == "addmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QString user=listwithheader[0].split(" ").at(0).trimmed();
            bool isTeraFly=listwithheader[0].split(" ").at(1).trimmed()=="TeraFly";
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                emit addMarker(listwithheader[1]);
            }
        }else if(operationtype == "delmarker")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QString user=listwithheader[0].split(" ").at(0).trimmed();
            bool isTeraFly=listwithheader[0].split(" ").at(1).trimmed()=="TeraFly";
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                emit delMarker(listwithheader[1]);
            }
        }else if(operationtype == "retypeline")
        {
            QString msg =operatorMsg;
            QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
            if(listwithheader.size()<=1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }

            QString user=listwithheader[0].split(" ").at(0).trimmed();
            bool isTeraFly=listwithheader[0].split(" ").at(1).trimmed()=="TeraFly";
            int type=listwithheader[0].split(" ").at(2).trimmed().toInt();
            if (user == userName && isNorm && isTeraFly)
                qDebug() << "user:" << user << "==userName" << userName;
            else
            {
                listwithheader.removeAt(0);
                emit retypeSeg(listwithheader.join(";"),type);
            }
        }
    }
}

void V3dR_Communicator::UpdateAddSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/drawline_norm:"+result.join(";")));
        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        undoDeque.push_back(QString("/delline_undo:"+result.join(";")));
        redoDeque.clear();
    }
}

void V3dR_Communicator::UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5")
        .arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/delline_norm:"+result.join(";")));
        while(undoDeque.size()>=dequeszie)
        {
            undoDeque.pop_front();
        }
        undoDeque.push_back(QString("/drawline_undo:"+result.join(";")));
        redoDeque.clear();

    }
}

void V3dR_Communicator::UpdateAddMarkerMsg(float X, float Y, float Z,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));

        XYZ global_node=ConvertLocalBlocktoGlobalCroods(X,Y,Z);
        result.push_back(QString("%1 %2 %3 %4").arg(type).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/addmarker_norm:"+result.join(";")));

    }
}
void V3dR_Communicator::UpdateDelMarkerSeg(float x,float y,float z,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        XYZ global_node=ConvertLocalBlocktoGlobalCroods(x,y,z);
        result.push_back(QString("%1 %2 %3 %4").arg(-1).arg(global_node.x).arg(global_node.y).arg(global_node.z));
        sendMsg(QString("/delmarker_norm:"+result.join(";")));

    }
}

void V3dR_Communicator::UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {

        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(userName).arg(clienttype).arg(type).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/retypeline_norm:"+result.join(";")));
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
//            else if("addmarker"==operationType)
//                operationType="/delmarker";
//            else if("delmarker"==operationType)
//                operationType="/addmarker";
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
//            else if("addmarker"==operationType)
//                operationType="/delmarker";
//            else if("delmarker"==operationType)
//                operationType="/addmarker";
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

void V3dR_Communicator::onConnected() {
    qDebug()<<"Message onConnected";
    sendMsg(QString("/login:" +userName));
    sendMsg("/GetBBSwc:");
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

//void V3dR_Communicator::onDisconnected() {
//    QMessageBox::information(0,tr("Message socket Connection is out!"),
//                     tr("Data has been safely stored!\nif it is not you disconnect.\nPlease restart vaa3d"),
//                     QMessageBox::Ok);
//    deleteLater();
//}

XYZ V3dR_Communicator::ConvertGlobaltoLocalBlockCroods(double x,double y,double z)
{
    auto node=ConvertMaxRes2CurrResCoords(x,y,z);
    node.x-=(ImageStartPoint.x-1);
    node.y-=(ImageStartPoint.y-1);
    node.z-=(ImageStartPoint.z-1);
    qDebug()<<"ConvertGlobaltoLocalBlockCroods x y z = "<<x<<" "<<y<<" "<<z<<" -> "+XYZ2String(node);
    return node;
}

XYZ V3dR_Communicator::ConvertLocalBlocktoGlobalCroods(double x,double y,double z)
{
    x+=(ImageStartPoint.x-1);
    y+=(ImageStartPoint.y-1);
    z+=(ImageStartPoint.z-1);
    XYZ node=ConvertCurrRes2MaxResCoords(x,y,z);
//    qDebug()<<"ConvertLocalBlocktoGlobalCroods x y z = "<<x<<" "<<y<<" "<<z<<" -> "+XYZ2String(node);
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
    x*=(ImageMaxRes.x/ImageCurRes.x);
    y*=(ImageMaxRes.y/ImageCurRes.y);
    z*=(ImageMaxRes.z/ImageCurRes.z);
    return XYZ(x,y,z);
}
