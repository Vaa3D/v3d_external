#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include <QRegExp>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>

V3dR_Communicator::V3dR_Communicator()
{
	CreatorMarkerPos = 0;
	CreatorMarkerRes = 0;
	userName="";

    socket = new QTcpSocket(this);
    resetDataInfo();
    connect(this,SIGNAL(msgtoprocess(QString)),this,SLOT(TFProcess(QString)));

}

void V3dR_Communicator::onReadyRead()
{
    QDataStream in(socket);
    if(dataInfo.dataSize==0)
    {
        if(socket->bytesAvailable()>=sizeof (qint32))
        {
            in>>dataInfo.dataSize;
            dataInfo.dataReadedSize+=sizeof (qint32);
        }
        else return;
    }

    if(dataInfo.stringOrFilenameSize==0&&dataInfo.filedataSize==0)
    {
        if(socket->bytesAvailable()>=2*sizeof (qint32))
        {
            in>>dataInfo.stringOrFilenameSize>>dataInfo.filedataSize;
            dataInfo.dataReadedSize+=(2*sizeof (qint32));
        }else
            return;
    }
    QStringList list;
    if(socket->bytesAvailable()>=dataInfo.stringOrFilenameSize+dataInfo.filedataSize)
    {
        QString messageOrFileName=QString::fromUtf8(socket->read(dataInfo.stringOrFilenameSize),dataInfo.stringOrFilenameSize);

        if(dataInfo.filedataSize)
        {
            if(!QDir(QCoreApplication::applicationDirPath()+"/loaddata").exists())
            {
                QDir(QCoreApplication::applicationDirPath()).mkdir("loaddata");
            }
            QString filePath=QCoreApplication::applicationDirPath()+"/loaddata/"+messageOrFileName;
            QFile file(filePath);
            file.open(QIODevice::WriteOnly);
            file.write(socket->read(dataInfo.filedataSize));file.flush();
            file.close();
            list.push_back("11"+filePath);
            emit this->load(filePath.section("/",-1));
        }else
        {
            list.push_back("00"+messageOrFileName);
        }
        dataInfo.dataReadedSize+=(dataInfo.stringOrFilenameSize+dataInfo.filedataSize);
        dataInfo.stringOrFilenameSize=0;
        dataInfo.filedataSize=0;
        if(dataInfo.dataReadedSize==dataInfo.dataSize)
            resetDataInfo();
        processReaded(list);
    }else
        return;
	onReadyRead();
}

void V3dR_Communicator::sendMsg(QString msg)
{
    qint32 stringSize=msg.toUtf8().size();
    qint32 totalsize=3*sizeof (qint32)+stringSize;
    QByteArray block;
    QDataStream dts(&block,QIODevice::WriteOnly);
    dts<<qint32(totalsize)<<qint32(stringSize)<<qint32(0);
    block+=msg.toUtf8();
    socket->write(block);
    socket->flush();
}

void V3dR_Communicator::processReaded(QStringList list)
{
    QStringList filepaths;
    for(auto msg:list)
    {
        if(msg.startsWith("00"))
        {
            emit msgtoprocess(msg.remove(0,2));
        }else if(msg.startsWith("11"))
        {
            filepaths.push_back(msg.remove(0,2));
        }
    }
    if(filepaths.size()==3)
    {
        if(filepaths[0].endsWith(".ano")&&filepaths[1].endsWith(".ano.apo")
                &&filepaths[2].endsWith(".ano.eswc"))
        {
            emit this->load(filepaths[0]);
        }
    }
}

void V3dR_Communicator::resetDataInfo()
{
     dataInfo.dataSize=0;dataInfo.stringOrFilenameSize=0;
     dataInfo.dataReadedSize=0;dataInfo.filedataSize=0;
}

void V3dR_Communicator::TFProcess(QString line,bool flag_init) {
    QRegExp usersRex("^/users:(.*)$");
    QRegExp hmdposRex("^/hmdpos:(.*)$");

    QRegExp drawlineRex("^/drawline:(.*)$");
    QRegExp dellineRex("^/delline:(.*)$");
    QRegExp addmarkerRex("^/addmarker:(.*)$");
    QRegExp delmarkerRex("^/delmarker:(.*)$");
    QRegExp retypelineRex("^/retype:(.*)$");

//    QRegExp dragnodeRex("^/drag_node:(.*)$");
//    QRegExp creatorRex("^/creator:(.*)__(.*)$");

    line=line.trimmed();
    qDebug()<<"Terafly receive:"<<line;
    if (usersRex.indexIn(line) != -1) {
        qDebug()<<"for now uses"<<usersRex.cap(1);
    }
    else if (drawlineRex.indexIn(line) != -1) {
        //line msg format:username clienttype RESx RESy RESz;type x y z;type x y z;...
        QString msg=drawlineRex.cap(1);
        QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
        if(listwithheader.size()<1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }
        QString user=listwithheader[0].split(" ").at(0).trimmed();
        if (user == userName)
            qDebug() << "user:" << user << "==userName" << userName;
        else
        {
            listwithheader.removeAt(0);
            emit addSeg(listwithheader.join(";"));
        }
    }else if (dellineRex.indexIn(line) != -1) {
        //line msg format:username clienttype RESx RESy RESz;type x y z;type x y z;...
        QString msg = dellineRex.cap(1);
        QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
        if(listwithheader.size()<1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }
        qDebug() << "+============delseg process begin========";
        QString user=listwithheader[0].split(" ").at(0).trimmed();
        if (user == userName)
            qDebug() << "user:" << user << "==userName" << userName;
        else
        {
            listwithheader.removeAt(0);
            emit delSeg(listwithheader.join(";"));
        }
    }else if (addmarkerRex.indexIn(line) != -1) {//Update by FJ 2020/6/14
        //marker msg format:username clienttype RESx RESy RESz;type x y z
        QString msg = addmarkerRex.cap(1);
        QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
        if(listwithheader.size()<1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }
        QString user=listwithheader[0].split(" ").at(0).trimmed();
        if (user == userName)
            qDebug() << "user:" << user << "==userName" << userName;
        else
        {
            emit addMarker(listwithheader[1]);
        }
    }else if (delmarkerRex.indexIn(line) != -1) {
        //marker msg format:username clienttype RESx RESy RESz;type x y z
        QString msg = delmarkerRex.cap(1);
        QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
        if(listwithheader.size()<1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }
        QString user=listwithheader[0].split(" ").at(0).trimmed();
        if (user == userName)
            qDebug() << "user:" << user << "==userName" << userName;
        else
        {
            emit delMarker(listwithheader[1]);
        }
    }else if(retypelineRex.indexIn(line)!=-1)
    {
        //line msg format:username clienttype  newtype RESx RESy RESz;type x y z;type x y z;...
        QString msg = delmarkerRex.cap(1);
        QStringList listwithheader=msg.split(';',QString::SkipEmptyParts);
        if(listwithheader.size()<=1)
        {
            qDebug()<<"msg only contains header:"<<msg;
            return;
        }

        QString user=listwithheader[0].split(" ").at(0).trimmed();
        if (user == userName)
            qDebug() << "user:" << user << "==userName" << userName;
        else
        {
            listwithheader.removeAt(0);
            emit retypeSeg(listwithheader.join(";"),listwithheader[2].split(" ").at(0).trimmed().toInt());
        }
    }/*else if(creatorRex.indexIn(line)!=-1)
    {
        //wait to implentment
    }*/
}

void V3dR_Communicator::UpdateAddSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/drawline:"+result.join(";")));
    }
}

void V3dR_Communicator::UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5").arg(userName).arg(clienttype).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/delline:"+result.join(";")));
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
        sendMsg(QString("/addmarker:"+result.join(";")));
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
        sendMsg(QString("/delmarker:"+result.join(";")));
    }
}

void V3dR_Communicator::UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype)
{
    if(clienttype=="TeraFly")
    {
        QStringList result;
        result.push_back(QString("%1 %2 %3 %4 %5 %6").arg(userName).arg(clienttype).arg(type).arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z));
        result+=V_NeuronSWCToSendMSG(seg);
        sendMsg(QString("/delline:"+result.join(";")));
    }
}

void V3dR_Communicator::onConnected() {
    sendMsg(QString("/login:" +userName));
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

void V3dR_Communicator::onDisconnected() {
    QMessageBox::information(this,tr("Message socket Connection is out!"),
                     tr("Data has been safely stored!\nif it is not you disconnect.\nPlease restart vaa3d"),
                     QMessageBox::Ok);
    deleteLater();
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
    XYZ node=ConvertLocalBlocktoGlobalCroods(x,y,z);

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

//XYZ V3dR_Communicator::ConvertLocaltoGlobalCroods(double x,double y,double z,XYZ* para)//for app2
//{
//    //Para={MaxRes, start_global,start_local}
//    x+=para[1].x-para[2].x;
//    y+=para[1].y-para[2].y;
//    z+=para[1].z-para[2].z;
//    return XYZ(x,y,z);
//}

//void V3dR_Communicator::read_autotrace(QString path,XYZ* tempPara)
//{
//    NeuronTree auto_res=readSWC_file(path);

//    V_NeuronSWC_list testVNL= NeuronTree__2__V_NeuronSWC_list(auto_res);

//    if(testVNL.seg.size()!=0)
//    {
//        for(int i=0;i<testVNL.seg.size();i++)
//        {

//            V_NeuronSWC seg_temp =  testVNL.seg.at(i);
////            qDebug()<<V_NeuronSWCToSendMSG(seg_temp,tempPara);
//            onReadySend(QString("/seg: "+V_NeuronSWCToSendMSG(seg_temp,tempPara)));
//        }
//    }
//}


//void V3dR_Communicator::pushUndoStack(QString head, QString Msg)
//{
//    if(undoStack.size()>=10)
//        undoStack.removeAt(0);
//    undoStack.push_back("/undo:"+Msg);
//}

//void V3dR_Communicator::undo()
//{
////    qDebug() << "--------------undo--------------"<<undoStack.size();
//    if(undoStack.size()>0)
//    {
//        qDebug()<<undoStack.at(undoStack.size()-1);
//        onReadySend(undoStack.at(undoStack.size()-1),0);
//        undoStack.removeAt(undoStack.size()-1);
//    }
//}

//QString V3dR_Communicator::V_NeuronSWCToSendMSG(V_NeuronSWC seg,XYZ* para)
//{
////    char extramsg[300];
//    QString messageBuff=QString("TeraAI %1 %2 %3_").arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);

//    reverse(seg.row.begin(),seg.row.end());
//    if(seg.row.size()>=3)
//    {
//        seg.row.erase(seg.row.end()-1);
//        seg.row.erase(seg.row.end()-1);
//        seg.row.erase(seg.row.end()-1);
//    }
//    for(int i=0;i<seg.row.size();i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
//    {
//        V_NeuronSWC_unit curSWCunit = seg.row[i];
//        QString packetbuff;
//        packetbuff.clear();
//        if(i!=seg.row.size()-1)
//        {
//            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z,para);

//            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7_").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
//                    .arg(curSWCunit.r).arg(curSWCunit.parent);

//        }
//        else
//        {
//            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z,para);
//            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
//                    .arg(curSWCunit.r).arg(curSWCunit.parent);
//            AutoTraceNode=XYZ(GlobalCroods.x,GlobalCroods.y,GlobalCroods.z);
//        }
//        messageBuff+=packetbuff;
//    }
//    if(seg.row.size()>4)
//    {
//        if(seg.row[seg.row.size()-1].x-seg.row[seg.row.size()-4].x>0) flag_x=1;else flag_x=-1;
//        if(seg.row[seg.row.size()-1].y-seg.row[seg.row.size()-4].y>0) flag_y=1;else flag_y=-1;
//        if(seg.row[seg.row.size()-1].z-seg.row[seg.row.size()-4].z>0) flag_z=1;else flag_z=-1;
//        //        qDebug()<<"flag(x,y,z)="<<flag_x<<" "<<flag_y<<" "<<flag_z;
//    }
//    return messageBuff;
//}

//void V3dR_Communicator::MsgToV_NeuronSWC(QString msg)
//{
//	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
//	int str_size = qsl.size()-(qsl.size()%3);//to make sure that the string list size always be 3*N;
//	vector<XYZ> loclist_temp;
//	XYZ loc_temp;
//	for(int i=0;i<str_size;i++)
//	{
//		qsl[i].truncate(99);
//		int iy = i%7;
//		if (iy==0)
//		{
//			loc_temp.x = qsl[i].toFloat();
//		}
//		else if(iy==1)
//		{
//			loc_temp.y = qsl[i].toFloat();
//		}
//		else if(iy==2)
//		{
//			loc_temp.z = qsl[i].toFloat();
//			loclist_temp.emplace_back(loc_temp);
//		}
//	}
//	loc_ReceivePool.push_back(loclist_temp);
//	//update AddCurveSWC
//}
