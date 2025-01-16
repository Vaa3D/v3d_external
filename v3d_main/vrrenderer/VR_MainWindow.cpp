#include "v3dr_gl_vr.h"
#include "VR_MainWindow.h"

#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>
#include <math.h>

//extern std::vector<Agent> Agents;
//std::vector<Agent> Agents;
VR_MainWindow::VR_MainWindow(V3dR_Communicator * TeraflyCommunicator) :
    QWidget()
{
    timerCheckConnVR = new QTimer(this);
    userId="";
    VR_Communicator = TeraflyCommunicator;
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtowarn(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtowarn(QString)), this, SLOT(processWarnMsg(QString)));
    disconnect(timerCheckConnVR, SIGNAL(timeout()), 0, 0);
    connect(timerCheckConnVR, SIGNAL(timeout()), this, SLOT(checkConnectionForVR()));
    timerCheckConnVR->start(3000);
    //    disconnect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), 0, 0);
    //    connect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), this, SLOT(processAnalyzeMsg(QString)));
    //    connect(this,SIGNAL(sendPoolHead()),this,SLOT(onReadySendSeg()));
    userId = TeraflyCommunicator->userId;
    qDebug()<<"userId "<<userId<<" "<<VR_Communicator->userId;
    CURRENT_DATA_IS_SENT=false;
}

VR_MainWindow::~VR_MainWindow() {
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtowarn(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtowarn(QString)), this, SLOT(processWarnMsg(QString)));
    //    disconnect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), 0, 0);
    //    connect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), this, SLOT(processAnalyzeMsg(QString)));
}

void VR_MainWindow::checkConnectionForVR(){
    if(!VR_Communicator->socket || VR_Communicator->socket->state() != QAbstractSocket::ConnectedState){
        QString msg = "Disconnection! Start Reconnecting...";
        qDebug() << msg;
        this->isQuit = true;

        //                if(cur_win->view3DWidget->myvrwin->pMainApplication){
        //                    cur_win->view3DWidget->myvrwin->shutdown();
        //                    delete cur_win->view3DWidget->myvrwin;
        //                    cur_win->view3DWidget->myvrwin=nullptr;
        //                }
        //            }
        //            doCollaborationVRView();
    }
}

//void VR_MainWindow::onReadySendSeg()
//{
//	if(!CollaborationSendPool.empty())
//	{
//		cout<<"CollaborationSendPool.size()"<<CollaborationSendPool.size()<<endl;
//		QString send_MSG = *CollaborationSendPool.begin();
//		CollaborationSendPool.erase(CollaborationSendPool.begin());
//		if((send_MSG!="exit")&&(send_MSG!="quit"))
//		{
//            VR_Communicator->sendMsg("/drawline:" + send_MSG);
//		}
//	}
//	else
//    {
//        cout<<"CollaborationSendPool is empty";
//	}
//}
//void VR_MainWindow::onReadyRead()
void VR_MainWindow::TVProcess(QString line)
{
    QRegExp msgreg("/(.*)_(.*):(.*)");
    line=line.trimmed();
    qDebug()<<line;
    if(msgreg.indexIn(line)!=-1)
    {
        QString operationtype=msgreg.cap(1).trimmed();
        bool isNorm=msgreg.cap(2).trimmed()=="norm";
        QString operatorMsg=msgreg.cap(3).trimmed();
        if(operationtype == "drawline" )
        {
            QString msg=operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",Qt::SkipEmptyParts);
            if(listwithheader.size()<1) return;

            QString user=listwithheader[0].split(" ").at(1).trimmed();
            //QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            int type=-1;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(listwithheader[i]=="$")
                        break;
                    auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_back(converted);

                    type=nodeinfo[0].toInt();
                }
                if(pMainApplication&&!coords.isEmpty())
                {
                    qDebug()<<type<<" "<<coords.size();
                    pMainApplication->UpdateNTList(coords,type);
                    //需要判断线是否在图像中，如果不在则调用全局处理
                }
                listwithheader.removeFirst();
                addCurveInAllSpace(listwithheader.join(","));
            }

            if(user==VR_Communicator->userId)
            {
                qDebug()<<"release lock";
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug()<<"user = "<<user<<" "<<userId;
            }
        }else if(operationtype == "delline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",Qt::SkipEmptyParts);
            //        qDebug()<<"list with header:"<<listwithheader;
            if(listwithheader.size()<1) return;
            // QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            QStringList infos=listwithheader[0].split(" ");
            QString user=infos.at(1).trimmed();
            unsigned int isMany=0;
            if(infos.size()>=6)
                isMany=infos.at(5).trimmed().toInt();

            QStringList coordsInSeg;

            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_front(converted);
                            coordsInSeg.append(listwithheader[i]);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                            {
                                qDebug()<<"delete in block failed";
                            }
                            VR_Communicator->emitDelSeg(coordsInSeg.join(","), 0);
                        }
                        coords.clear();
                        coordsInSeg.clear();
                        //                        deleteCurveInAllSpace()
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInSeg.append(listwithheader[i]);
                }
            }

            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;

            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "splitline"){
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',Qt::SkipEmptyParts);
            qDebug()<<"splitline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QStringList infos=listwithheader[0].split(" ");
            QString user=infos.at(1).trimmed();
            QStringList coordsInDelSeg;
            int index=0;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(listwithheader[i]=="$"){
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                            {
                                qDebug()<<"delete in block failed";
                            }
                            VR_Communicator->emitDelSeg(coordsInDelSeg.join(","), 0);
                        }
                        coords.clear();
                        coordsInDelSeg.clear();
                        index = i+1;
                        break;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInDelSeg.append(listwithheader[i]);
                }

            }

            QStringList coordsInAddSeg;
            int type=-1;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=index;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_back(converted);
                            coordsInAddSeg.append(listwithheader[i]);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            qDebug()<<type<<" "<<coords.size();
                            pMainApplication->UpdateNTList(coords,type);
                            //需要判断线是否在图像中，如果不在则调用全局处理
                        }
                        addCurveInAllSpace(coordsInAddSeg.join(","));
                        coords.clear();
                        coordsInAddSeg.clear();
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_back(converted);
                    coordsInAddSeg.append(listwithheader[i]);
                    type=nodeinfo[0].toInt();
                }
            }

            if(user==VR_Communicator->userId)
            {
                qDebug()<<"release lock";
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug()<<"user = "<<user<<" "<<userId;
            }

        }else if(operationtype == "addmarker")
        {
            QString msg = operatorMsg;
            qDebug()<<"TeraVR add marker";
            QStringList listwithheader=operatorMsg.split(",",Qt::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',Qt::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                auto node=listwithheader[1].split(" ");
                RGB8 color;
                color.r = node.at(0).toUInt();
                color.g = node.at(1).toUInt();
                color.b = node.at(2).toUInt();
                float mx = node.at(3).toFloat();
                float my = node.at(4).toFloat();
                float mz = node.at(5).toFloat();
                XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                //需要判断点是否在图像中，如果不在则全局处理
                VR_Communicator->emitAddMarker(listwithheader[1], "");
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "delmarker")
        {
            qDebug()<<"TeraVR del marker";
            QStringList listwithheader=operatorMsg.split(",",Qt::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',Qt::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                for(int i=1; i<listwithheader.size(); i++){
                    auto node=listwithheader[i].split(" ");
                    RGB8 color;
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
                    VR_Communicator->emitDelMarker(listwithheader[i]);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "retypeline")
        {
            QString msg =operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",Qt::SkipEmptyParts);
            if(listwithheader.size()<1) return;
            QStringList infos=listwithheader[0].split(" ");


            QString user=listwithheader[0].trimmed().split(' ',Qt::SkipEmptyParts)[1].trimmed();
            int type=listwithheader[0].trimmed().split(' ',Qt::SkipEmptyParts)[2].trimmed().toInt();
            unsigned int isMany=0;
            if(infos.size()>=7)
                isMany=infos.at(6).trimmed().toInt();

            QStringList coordsInSeg;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_front(converted);
                            coordsInSeg.append(listwithheader[i]);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->retypeSegment(
                                    coords,
                                    1.0*VRVolumeCurrentRes.x/VRvolumeMaxRes.x,
                                    type))
                            {
                                qDebug()<<"Vr call fly retype ";
                            }
                            VR_Communicator->emitRetypeSeg(coordsInSeg.join(","),type,0);
                        }
                        coords.clear();
                        coordsInSeg.clear();
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",Qt::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInSeg.append(listwithheader[i]);
                }

            }

            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }
    }
}

void VR_MainWindow::processWarnMsg(QString line){
    QRegExp warnreg("/WARN_(.*):(.*)");
    line=line.trimmed();
    if(warnreg.indexIn(line)!=-1)
    {
        QString reason=warnreg.cap(1).trimmed();
        QString operatorMsg=warnreg.cap(2).trimmed();
        QString msg = operatorMsg;
        QStringList listwithheader=msg.split(',',Qt::SkipEmptyParts);
        //        if(listwithheader.size()<2)
        //        {
        //            qDebug()<<"msg only contains header:"<<msg;
        //            return;
        //        }

        QString header = listwithheader[0];
        QString sender=header.split(" ").at(0).trimmed();
        listwithheader.removeAt(0);

        if(sender=="server"){
            if(reason=="TipUndone" || reason=="CrossingError" || reason=="MulBifurcation" || reason=="BranchingError")
            {
                QString comment;
                if(reason == "TipUndone"){
                    comment = "Missing";
                }else if(reason == "CrossingError"){
                    comment = "Crossing error";
                }else if(reason == "MulBifurcation"){
                    comment = "Multifurcation";
                }else if(reason == "BranchingError"){
                    comment = "Branching error";
                }
                if(listwithheader.size()>=1)
                {
                    for(int i=0; i<listwithheader.size(); i++){
                        auto node=listwithheader[i].split(" ");
                        RGB8 color;
                        color.r = node.at(0).toUInt();
                        color.g = node.at(1).toUInt();
                        color.b = node.at(2).toUInt();
                        float mx = node.at(3).toFloat();
                        float my = node.at(4).toFloat();
                        float mz = node.at(5).toFloat();
                        XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                        //需要判断点是否在图像中，如果不在则全局处理
                    }
                    VR_Communicator->emitAddManyMarkers(listwithheader.join(","), comment);
                }
            }
            else if(reason=="Loop")
            {
                int result = header.split(" ").at(1).trimmed().toUInt();
                if(result == 1){
                    //                emit setDefineSomaActionState(true);
                }
                if(result == 0){
                    //                emit setDefineSomaActionState(false);
                    for(int i=0; i<listwithheader.size(); i++){
                        auto node=listwithheader[i].split(" ");
                        RGB8 color;
                        color.r = node.at(0).toUInt();
                        color.g = node.at(1).toUInt();
                        color.b = node.at(2).toUInt();
                        float mx = node.at(3).toFloat();
                        float my = node.at(4).toFloat();
                        float mz = node.at(5).toFloat();
                        XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                        //需要判断点是否在图像中，如果不在则全局处理
                    }
                    VR_Communicator->emitAddManyMarkers(listwithheader.join(","), "Loop");
                }
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
            }
            else if(reason=="ApoFileNotFoundError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ApoFileNotFoundError!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetApoDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetApoDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetSwcFullNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetSwcFullNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="AddSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("AddSwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="DeleteSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("DeleteSwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="ModifySwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ModifySwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="UpdateSwcAttachmentApoError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("UpdateSwcAttachmentApoError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="FullNumberError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("The number of collaborating users has been full about this swc!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="Approaching bifurcation"){
                auto node=listwithheader[0].split(" ");
                RGB8 color;
                color.r = node.at(0).toUInt();
                color.g = node.at(1).toUInt();
                color.b = node.at(2).toUInt();
                float mx = node.at(3).toFloat();
                float my = node.at(4).toFloat();
                float mz = node.at(5).toFloat();
                XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                VR_Communicator->emitAddMarker(listwithheader[0], "Approaching bifurcation");
            }
        }
    }
}

//void VR_MainWindow::processAnalyzeMsg(QString line){
//    qDebug()<<line;
//    QRegExp analyzereg("/FEEDBACK_ANALYZE_(.*):(.*)");
//    QRegExp definereg("/FEEDBACK_DEFINE_(.*):(.*)");
//    line=line.trimmed();
//    if(analyzereg.indexIn(line) != -1)
//    {
//        QString type=analyzereg.cap(1).trimmed();
//        QString operatorMsg=analyzereg.cap(2).trimmed();

//        qDebug()<<"type:"<<type;
//        qDebug()<<"operatormsg:"<<operatorMsg;
//        if(type=="SomaNearBy"){
//            QString sender=operatorMsg.split(" ").at(0).trimmed();
//            QString request_senderid=operatorMsg.split(" ").at(1).trimmed();
//            int result=operatorMsg.split(" ").at(2).trimmed().toInt();
//            if (sender=="server" && result==1)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("no error"),
//                                         QMessageBox::Ok);
//            }
//            else if(sender=="server" && result==0){
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("error: soma is not connected to one point!"),
//                                         QMessageBox::Ok);
//            }else if(sender=="server" && result==-1){
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("error: soma not detected!"),
//                                         QMessageBox::Ok);
//            }
//        }
//        if(type=="ColorMutation"){
//            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
//            QString msgHeader=listWithHeader[0];
//            QStringList msgList=listWithHeader;
//            msgList.removeAt(0);

//            QString sender=msgHeader.split(" ").at(0).trimmed();
//            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
//            int result=msgHeader.split(" ").at(2).trimmed().toInt();
//            if (sender=="server" && result==1)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("no error"),
//                                         QMessageBox::Ok);
//            }
//            else if(sender=="server" && result==0){
//                emit addManyMarkers(msgList.join(","), "Color mutation");

//                emit updateQcInfo();
//                emit updateQcMarkersCounts();

//                if( request_senderid==userId )
//                    QMessageBox::information(0,tr("Infomation "),
//                                             tr("error: color mutation exists! notice the soma nearby and the red markers!"),
//                                             QMessageBox::Ok);
//            }else if(sender=="server" && result==-1){
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("error: soma not detected!"),
//                                         QMessageBox::Ok);
//            }
//        }
//        if(type=="Dissociative"){
//            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
//            QString msgHeader=listWithHeader[0];
//            QStringList msgList=listWithHeader;
//            msgList.removeAt(0);

//            QString sender=msgHeader.split(" ").at(0).trimmed();
//            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
//            int result=msgHeader.split(" ").at(2).trimmed().toInt();
//            if (sender=="server" && result==1)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("no error"),
//                                         QMessageBox::Ok);
//            }
//            else if(sender=="server" && result==0){
//                emit addManyMarkers(msgList.join(","), "Dissociative seg");

//                emit updateQcInfo();
//                emit updateQcMarkersCounts();

//                if(request_senderid==userId)
//                    QMessageBox::information(0,tr("Infomation "),
//                                             tr("error: dissociative seg exists! notice the red markers!"),
//                                             QMessageBox::Ok);
//            }
//        }
//        if(type=="Angle"){
//            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
//            QString msgHeader=listWithHeader[0];
//            QStringList msgList=listWithHeader;
//            msgList.removeAt(0);

//            QString sender=msgHeader.split(" ").at(0).trimmed();
//            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
//            int result=msgHeader.split(" ").at(2).trimmed().toInt();
//            if (sender=="server" && result==1)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("no error"),
//                                         QMessageBox::Ok);
//            }
//            else if(sender=="server" && result==0){
//                emit addManyMarkers(msgList.join(","), "Angle error");

//                emit updateQcInfo();
//                emit updateQcMarkersCounts();

//                if(request_senderid==userId)
//                    QMessageBox::information(0,tr("Infomation "),
//                                             tr("error: incorrect angle exists! notice the red markers!"),
//                                             QMessageBox::Ok);
//            }else if(sender=="server" && result==-1){
//                QMessageBox::information(0,tr("Infomation "),
//                                         tr("error: soma not detected!"),
//                                         QMessageBox::Ok);
//            }
//        }
//    }
//    else if(definereg.indexIn(line) != -1){
//        QString type=definereg.cap(1).trimmed();
//        QString operatorMsg=definereg.cap(2).trimmed();

//        qDebug()<<"type:"<<type;
//        qDebug()<<"operatormsg:"<<operatorMsg;
//        if(type=="Soma"){
//            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
//            QString msgHeader=listWithHeader[0];
//            QStringList msgList=listWithHeader;
//            msgList.removeAt(0);

//            QString sender=msgHeader.split(" ").at(0).trimmed();
//            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
//            int result=msgHeader.split(" ").at(2).trimmed().toInt();

//            if (sender=="server" && result==0)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         "error: " + msgList.join(","),
//                                         QMessageBox::Ok);
//            }
//            if(sender=="server" && result==1)
//            {
//                QMessageBox::information(0,tr("Infomation "),
//                                         msgList.join(","),
//                                         QMessageBox::Ok);
//            }
//        }
//    }
//}

void VR_MainWindow::processAnalyzeMsg(QString line){
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

        if(type=="ColorMutation"){
            QStringList listWithHeader=operatorMsg.split(",",Qt::SkipEmptyParts);
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
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
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
            QStringList listWithHeader=operatorMsg.split(",",Qt::SkipEmptyParts);
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
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
                if(request_senderid==userId)
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: Isolated branch exists! notice the red markers!"),
                                             QMessageBox::Ok);
            }
        }
        if(type=="Angle"){
            QStringList listWithHeader=operatorMsg.split(",",Qt::SkipEmptyParts);
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
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
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
}

int VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,
                                bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator* TeraflyCommunicator,
                                XYZ* zoomPOS,XYZ *CreatorPos,XYZ MaxResolution) {

    pMainApplication = new CMainApplication(  0, 0 ,TeraflyCommunicator->CreatorMarkerPos);

    pMainApplication->mainwindow =pmain;

    pMainApplication->isOnline = isLinkSuccess;
    GetResindexandStartPointfromVRInfo(ImageVolumeInfo,MaxResolution);
    if(ntlist != NULL)
    {
        if((ntlist->size()==1)&&(ntlist->at(0).name.isEmpty()))
        {
            // means there is only a reloaded annotation in terafly
            // we rename it as vaa3d_traced_neuron
            qDebug()<<"means this is terafly special condition.do something";
            NeuronTree newS;
            newS.color = XYZW(0,0,255,255);
            newS = ntlist->at(0);
            newS.n = 1;
            newS.on = true;
            newS.name = "vaa3d_traced_neuron";
            newS.file = "vaa3d_traced_neuron";
            pMainApplication->editableLoadedNTL.append(newS);
        }
        else
        {
            for(int i=0;i<ntlist->size();i++)
            {
                if((ntlist->at(i).name == "vaa3d_traced_neuron")&&(ntlist->at(i).file == "vaa3d_traced_neuron"))
                {
                    // means there is a NT named "vaa3d_traced_neuron", we only need to edit this NT.
                    pMainApplication->editableLoadedNTL.append(ntlist->at(i));
                }
                else if (!ntlist->at(0).name.isEmpty())
                {
                    // means it is a loaded Neuron in 3D View,currently we do not allow to edit this neuron in VR
                    pMainApplication->nonEditableLoadedNTL.append(ntlist->at(i));
                }
                // else if (ntlist->at(0).name.isEmpty())
                // means it is an reloaded annotation in terafly, currently we do not show this neuron in VR
            }
        }
    }
    pMainApplication->loadedNTList = ntlist;
    if(i4d->valid())
    {
        pMainApplication->img4d = i4d;
        pMainApplication->m_bHasImage4D=true;
    }
    if (!pMainApplication->BInit())
    {
        pMainApplication->Shutdown();
        qDebug()<<"init failed";
        return 0;
    }
    SendVRconfigInfo();
    pMainApplication->SetupCurrentUserInformation(VR_Communicator->userId.toStdString(),0);
    RunVRMainloop(zoomPOS);
    qDebug()<<"11111111111111111111111";
    pMainApplication->Shutdown();
    qDebug()<<"Now quit VR";
    int _call_that_function = pMainApplication->postVRFunctionCallMode;
    zoomPOS->x = pMainApplication->teraflyPOS.x;
    zoomPOS->y = pMainApplication->teraflyPOS.y;
    zoomPOS->z = pMainApplication->teraflyPOS.z;

    CreatorPos->x = VR_Communicator->CreatorMarkerPos.x;
    CreatorPos->y = VR_Communicator->CreatorMarkerPos.y;
    CreatorPos->z = VR_Communicator->CreatorMarkerPos.z;
    CreatorRes = VR_Communicator->CreatorMarkerRes;
    qDebug()<<"call that function is"<<_call_that_function;
    delete pMainApplication;
    pMainApplication=0;
    return _call_that_function;
}

void VR_MainWindow::shutdown(){
    pMainApplication->Shutdown();
    delete pMainApplication;
    pMainApplication=0;
}

//void VR_MainWindow::SendHMDPosition()
//{
//	if(!pMainApplication) return;
//	//get hmd position
//	QString PositionStr=pMainApplication->getHMDPOSstr();

//	//send hmd position
//    //VR_Communicator->onReadySend(QString("/hmdpos:" + PositionStr));
//	//QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
//	//cout<<"socket resindex"<<ResIndex<<endl;
//	//qDebug()<<"QString resindex"<< QString("%1").arg(ResIndex);
//    //VR_Communicator->onReadySend(QString("/ResIndex:" + QString("%1").arg(ResIndex) ));
//}
void VR_MainWindow::RunVRMainloop(XYZ* zoomPOS)
{
    qDebug()<<"get into RunMainloop";
    bool bQuit = false;
    while(!bQuit && !isQuit)
    {
        //handle one rendering loop, and handle user interaction
        //        qDebug()<<"VR_MainWindow"
        bQuit=pMainApplication->HandleOneIteration();
        //READY_TO_SEND is set to true by the "trigger button up" event;
        //client sends data to server (using onReadySend());
        //server sends the same data back to client;
        //READY_TO_SEND is set to false in onReadyRead();
        //CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
        qDebug()<<pMainApplication->READY_TO_SEND<<" "<<CURRENT_DATA_IS_SENT;
        if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
        {
            if(pMainApplication->undo)
            {
                if(VR_Communicator->undoDeque.size())
                {
                    VR_Communicator->UpdateUndoDeque();
                    CURRENT_DATA_IS_SENT=true;
                }
                else{
                    pMainApplication->TriggerHapticPluse();
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                pMainApplication->undo=false;
            }else if(pMainApplication->redo)
            {
                if(VR_Communicator->redoDeque.size())
                {
                    VR_Communicator->UpdateRedoDeque();
                    CURRENT_DATA_IS_SENT=true;
                }
                else{
                    pMainApplication->TriggerHapticPluse();
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }

                pMainApplication->redo=false;
            }

        }

        if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
        {
            if(pMainApplication->m_modeGrip_R==m_drawMode)
            {
                qDebug()<<"TeraVR add seg";
                QStringList waitsend=pMainApplication->NT2QString(pMainApplication->currentNT);
                if(waitsend.size())
                {

                    pMainApplication->ClearCurrentNT();
                    if(VR_Communicator&&VR_Communicator->socket&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                        waitsend.append("$");
                        VR_Communicator->UpdateAddSegMsg(waitsend.join(","));
                        CURRENT_DATA_IS_SENT=true;
                    }
                }else
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                }
            }
            else if(pMainApplication->m_modeGrip_R==m_deleteMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
                {
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&VR_Communicator->socket&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateDelSegMsg(QString(result.join(",")));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR del seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    qDebug()<<"TeraVR del seg failed";
                }
            }
            else if(pMainApplication->m_modeGrip_R==m_markMode)
            {
                if(pMainApplication->markerPosTobeDeleted!="")
                {
                    QStringList result;
                    QString ConvertedmarkerPOS = ConvertToMaxGlobalForMarker(pMainApplication->markerPosTobeDeleted);
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    result.push_back(ConvertedmarkerPOS);
                    if(ConvertedmarkerPOS.split(" ")[0]=="-1")
                    {
                        qDebug()<<"TeraVR del marker";
                        if(VR_Communicator&&VR_Communicator->socket&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            VR_Communicator->UpdateDelMarkerMsg(QString(result.join(",")));
                        }
                    }else
                    {
                        qDebug()<<"TeraVR add marker";
                        if(VR_Communicator&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            VR_Communicator->UpdateAddMarkerMsg(QString(result.join(",") ));
                        }
                    }
                    pMainApplication->markerPosTobeDeleted.clear();
                    CURRENT_DATA_IS_SENT=true;
                }else
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->markerPosTobeDeleted="";
                }
            }else if(pMainApplication->m_modeGrip_R==m_retypeMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
                {
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4 %5").arg(VR_Communicator->userId).arg(pMainApplication->m_curMarkerColorType).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&VR_Communicator->socket&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateRetypeSegMsg(QString(result.join(",")));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR retype seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    qDebug()<<"TeraVR retype seg failed";
                }
            }else if(pMainApplication->m_modeGrip_R == m_splitMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0 )
                {
                    if(pMainApplication->segaftersplit.size()!=2)
                    {
                        pMainApplication->READY_TO_SEND=false;
                        CURRENT_DATA_IS_SENT=false;
                        pMainApplication->ClearCurrentNT();
                        pMainApplication->segaftersplit.clear();
                        qDebug()<<"TeraVR del seg failed";
                    }
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    qDebug()<<"result = "<<result;
                    QStringList waitsends;
                    for(auto nt:pMainApplication->segaftersplit)
                    {
                        QStringList waitsend=pMainApplication->NT2QString(nt);
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                        waitsend.append("$");
                        waitsends.push_back(waitsend.join(","));
                    }
                    pMainApplication->segaftersplit.clear();
                    qDebug()<<"waitsends = "<<waitsends;


                    if(VR_Communicator&&VR_Communicator->socket&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        //                    VR_Communicator->UpdateDelSegMsg(QString(result.join(";")));
                        //                    for(auto addmsg:waitsends)
                        //                    {
                        //                        VR_Communicator->UpdateAddSegMsg(addmsg);
                        //                    }
                        //
                        VR_Communicator->UpdateSplitSegMsg(QString(result.join(",")),waitsends.at(0),waitsends.at(1));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR del seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    pMainApplication->segaftersplit.clear();
                    qDebug()<<"TeraVR del seg failed";
                }
            }
        }
    }

}

//-----------------------------------------------------------------------------
// Purpose: for standalone VR.
//-----------------------------------------------------------------------------
int startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain, XYZ* zoomPOS)
// bool startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain)
{

    CMainApplication *pMainApplication = new CMainApplication( 0, 0 );
    //pMainApplication->setnetworkmodefalse();//->NetworkModeOn=false;
    pMainApplication->mainwindow = pmain;
    pMainApplication->isOnline = false;


    if(ntlist != NULL)
    {
        if((ntlist->size()==1)&&(ntlist->at(0).name.isEmpty()))
        {
            // means there is only a reloaded annotation in terafly
            // we rename it as vaa3d_traced_neuron
            qDebug()<<"means this is terafly special condition.do something";
            NeuronTree newS;
            newS.color = XYZW(0,0,255,255);
            newS = ntlist->at(0);
            newS.n = 1;
            newS.on = true;
            newS.name = "vaa3d_traced_neuron";
            newS.file = "vaa3d_traced_neuron";
            pMainApplication->editableLoadedNTL.append(newS);
        }
        else
        {
            for(int i=0;i<ntlist->size();i++)
            {
                if((ntlist->at(i).name == "vaa3d_traced_neuron")&&(ntlist->at(i).file == "vaa3d_traced_neuron"))
                {
                    // means there is a NT named "vaa3d_traced_neuron", we only need to edit this NT.
                    pMainApplication->editableLoadedNTL.append(ntlist->at(i));
                }
                else if (!ntlist->at(0).name.isEmpty())
                {
                    // means it is a loaded Neuron in 3D View,currently we do not allow to edit this neuron in VR
                    pMainApplication->nonEditableLoadedNTL.append(ntlist->at(i));
                }
                // else if (ntlist->at(0).name.isEmpty())
                // means it is an reloaded annotation in terafly, currently we do not show this neuron in VR
            }
        }
    }
    pMainApplication->loadedNTList = ntlist;

    if(i4d->valid())
    {
        pMainApplication->img4d = i4d;
        pMainApplication->m_bHasImage4D=true;
    }
    if (!pMainApplication->BInit())
    {
        pMainApplication->Shutdown();
        return 0;
    }
    pMainApplication->SetupCurrentUserInformation("local user", 13);

    pMainApplication->RunMainLoop();

    pMainApplication->Shutdown();

    // bool _call_that_plugin = pMainApplication->_call_assemble_plugin;
    int _call_that_function = pMainApplication->postVRFunctionCallMode;
    zoomPOS->x = pMainApplication->teraflyPOS.x;
    zoomPOS->y = pMainApplication->teraflyPOS.y;
    zoomPOS->z = pMainApplication->teraflyPOS.z;
    delete pMainApplication;
    pMainApplication = NULL;

    // return _call_that_plugin;
    return _call_that_function;
}
void VR_MainWindow::GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution)
{
    qDebug() << "GetResindexandStartPointfromVRInfo........";
    qDebug()<<"VRinfo"<<VRinfo;
    QRegExp rx("Res\\((\\d+)\\s.\\s(\\d+)\\s.\\s(\\d+)\\),Volume\\sX.\\[(\\d+),(\\d+)\\],\\sY.\\[(\\d+),(\\d+)\\],\\sZ.\\[(\\d+),(\\d+)\\]");
    if (rx.indexIn(VRinfo) != -1 && (ResIndex != -1)) {
        qDebug()<<"get  VRResindex and VRVolume Start point ";
        VRVolumeStartPoint = XYZ(rx.cap(4).toInt(),rx.cap(6).toInt(),rx.cap(8).toInt());
        VRVolumeEndPoint = XYZ(rx.cap(5).toInt(),rx.cap(7).toInt(),rx.cap(9).toInt());
        VRVolumeCurrentRes = XYZ(rx.cap(1).toInt(),rx.cap(2).toInt(),rx.cap(3).toInt());
        VRvolumeMaxRes = CollaborationMaxResolution;

    }
    else
    {
        VRVolumeStartPoint = XYZ(1,1,1);
        VRVolumeEndPoint = CollaborationMaxResolution;
        VRVolumeCurrentRes = CollaborationMaxResolution;
        VRvolumeMaxRes = CollaborationMaxResolution;

    }
    //pass Resindex and VRvolumeStartPoint to PMAIN  to  offer parameter to NT2QString
    pMainApplication->collaborationTargetdelcurveRes = VRvolumeMaxRes;
    pMainApplication->CmainResIndex = ResIndex;
    pMainApplication->CmainVRVolumeStartPoint = VRVolumeStartPoint;
    pMainApplication->CmainVRVolumeEndPoint=VRVolumeEndPoint;


    pMainApplication->CollaborationMaxResolution = CollaborationMaxResolution;
    pMainApplication->CollaborationCurrentRes = VRVolumeCurrentRes;

}

QString VR_MainWindow::ConvertToMaxGlobalForMarker(QString coords)
{
    float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
    float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
    float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
    int r=coords.section(' ',3, 3).toInt();
    int g=coords.section(' ',4, 4).toInt();
    int b=coords.section(' ',5, 5).toInt();
    x+=(VRVolumeStartPoint.x-1);
    y+=(VRVolumeStartPoint.y-1);
    z+=(VRVolumeStartPoint.z-1);
    x*=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y*=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z*=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    return QString("%1 %2 %3 %4 %5 %6").arg(r).arg(g).arg(b).arg(x).arg(y).arg(z);
}

QString VR_MainWindow::ConvertToMaxGlobalForSeg(QString coords){
    float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
    float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
    float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
    int type = coords.section(' ',3, 3).toInt();
    x+=(VRVolumeStartPoint.x-1);
    y+=(VRVolumeStartPoint.y-1);
    z+=(VRVolumeStartPoint.z-1);
    x*=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y*=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z*=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    return QString("%1 %2 %3 %4").arg(type).arg(x).arg(y).arg(z);
}

void VR_MainWindow::SendVRconfigInfo()
{
    //float globalscale = pMainApplication->GetGlobalScale();
    //QString QSglobalscale = QString("%1").arg(globalscale);
    //   VR_Communicator->onReadySend(QString("/scale:" +  QSglobalscale ));
}

XYZ VR_MainWindow:: ConvertBlock2GloabelInRES(XYZ local)
{
    return XYZ(local.x+VRVolumeStartPoint.x,local.y+VRVolumeStartPoint.y,local.z+VRVolumeStartPoint.z);
}
XYZ VR_MainWindow:: ConvertMaxGlobal2LocalBlock(float x,float y,float z)
{
    //QString str1 = coords.section(' ',0, 0);  // str == "bin/myapp"
    //QString str2 = coords.section(' ',1, 1);  // str == "bin/myapp"
    //QString str3 = coords.section(' ',2, 2);  // str == "bin/myapp"
    x/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    x-=(VRVolumeStartPoint.x-1);
    y-=(VRVolumeStartPoint.y-1);
    z-=(VRVolumeStartPoint.z-1);
    return XYZ(x,y,z);
}

void VR_MainWindow::addCurveInAllSpace(QString segInfo){
    //    qDebug()<<"enter";
    if(segInfo.isEmpty()) return;
    NeuronTree nt = terafly::PluginInterface::getSWC();
    V_NeuronSWC_list v_ns_list=NeuronTree__2__V_NeuronSWC_list(nt);

    NeuronTree newTempNT;
    newTempNT.listNeuron.clear();
    newTempNT.hashNeuron.clear();
    QStringList qsl=segInfo.split(",",Qt::SkipEmptyParts);
    //    qDebug()<<"after receive the msg"<<segInfo;
    int index=0;
    int timestamp=QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i<qsl.size(); i++)
    {
        if(qsl[i]!="$"){
            NeuronSWC S;
            QStringList nodelist=qsl[i].split(" ",Qt::SkipEmptyParts);
            //            qDebug()<<i<<":"<<nodelist;
            if(nodelist.size()<4) return;
            S.n=i+1;
            S.type=nodelist[0].toInt();
            S.x=nodelist[1].toFloat();
            S.y=nodelist[2].toFloat();
            S.z=nodelist[3].toFloat();
            S.r=1;
            if(index==0) S.pn=-1;
            else S.pn=i;
            S.timestamp=timestamp;
            newTempNT.listNeuron.push_back(S);
            newTempNT.hashNeuron.insert(S.n,newTempNT.listNeuron.size());
            index++;
        }
        else{
            index=0;
        }
    }

    //        qDebug()<<"new NT is constructed";
    auto segs=NeuronTree__2__V_NeuronSWC_list(newTempNT).seg;
    v_ns_list.seg.push_back(segs[0]);
    nt=V_NeuronSWC_list__2__NeuronTree(v_ns_list);
    terafly::PluginInterface::setSWC(nt,true);
    //    qDebug()<<"end";
    //    QString fileName = "";
    //    writeSWC_file(fileName,nt);
}

void VR_MainWindow::deleteCurveInAllSpace(QString segInfo, int isMany){
    if(segInfo.isEmpty())
        return;
    NeuronTree  nt = terafly::PluginInterface::getSWC();
    V_NeuronSWC_list v_ns_list=NeuronTree__2__V_NeuronSWC_list(nt);
    //    qDebug()<<"ZLL________________1";

    auto segInfos=segInfo.split(",");

    float mindist=1;

    QVector<XYZ> coords;
    for(int i=0;i<segInfos.size();i++)
    {
        if(segInfos.at(i)!="$"){
            auto node= segInfos.at(i).split(" ");
            coords.push_back(XYZ(node[1].toFloat(),node[2].toFloat(),node[3].toFloat()));
        }
        else{
            int index=findseg(v_ns_list,coords);
            if(index>=0)
            {
                //        qDebug()<<"ZLL_____________________2.5";
                v_ns_list.seg.erase(v_ns_list.seg.begin()+index);
            }else
            {
                //                qDebug()<<"ERROR:cannot delete curve " + segInfo;
            }
            coords.clear();
        }
    }

    if(isMany==0){
        int index=findseg(v_ns_list,coords);
        if(index>=0)
        {
            //        qDebug()<<"ZLL_____________________2.5";
            v_ns_list.seg.erase(v_ns_list.seg.begin()+index);
        }else
        {
            //            qDebug()<<"ERROR:cannot delete curve " + segInfo;
        }
    }

    nt=V_NeuronSWC_list__2__NeuronTree(v_ns_list);
    terafly::PluginInterface::setSWC(nt,true);
}

//void VR_MainWindow::retypeCurveInAllSpace(QString segInfo,int type, int isMany);
//void VR_MainWindow::splitCurveInAllSpace(QString segInfo);
//void VR_MainWindow::delMarkersInAllSpace(QString markersPOS);
//void VR_MainWindow::addMarkerInAllSpace(QString markerPOS, QString comment);
int VR_MainWindow::findseg(V_NeuronSWC_list v_ns_list,QVector<XYZ> coords)
{
    double mindist = 1;
    int index=-1;
    //    for(int j=0;j<coords.size();j++)
    //    {
    //        qDebug()<<j<<":"<<coords[j].x<<" "<<coords[j].y<<" "<<coords[j].z;
    //    }
    for(int i=0;i<v_ns_list.seg.size();i++)
    {
        if(coords.size()!=v_ns_list.seg.at(i).row.size()) continue;
        auto seg=v_ns_list.seg.at(i).row;
        //        for(int j=0;j<coords.size();j++)
        //        {
        //            qDebug()<<j<<":"<<seg[j].x<<" "<<seg[j].y<<" "<<seg[j].z;
        //        }
        float sum=0;
        for(int j=0;j<coords.size();j++)
        {
            sum+=sqrt(pow(coords[j].x-seg[j].x,2)+pow(coords[j].y-seg[j].y,2)
                        +pow(coords[j].z-seg[j].z,2));
        }
        if(sum/coords.size()<mindist)
        {
            mindist=sum/coords.size();
            index=i;
        }
        std::reverse(coords.begin(),coords.end());
        sum=0;
        for(int j=0;j<coords.size();j++)
        {
            sum+=sqrt(pow(coords[j].x-seg[j].x,2)+pow(coords[j].y-seg[j].y,2)
                        +pow(coords[j].z-seg[j].z,2));
        }
        if(sum/coords.size()<mindist)
        {
            mindist=sum/coords.size();
            index=i;
        }
    }
    //    if(index<0) qDebug()<<"fail to findseg";
    return index;
}
