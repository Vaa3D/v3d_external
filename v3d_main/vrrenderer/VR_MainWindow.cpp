#include "v3dr_gl_vr.h"
#include "VR_MainWindow.h"


#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>
#include <math.h>
#include <fstream>
#include "shader_m.h"
//extern std::vector<Agent> Agents;
//std::vector<Agent> Agents;
VR_MainWindow::VR_MainWindow(V3dR_Communicator * TeraflyCommunicator) :
	QWidget()
{

	userName="";
	QRegExp regex("^[a-zA-Z]\\w+");
//	socket = new QTcpSocket(this);
	VR_Communicator = TeraflyCommunicator;
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
	connect(this,SIGNAL(sendPoolHead()),this,SLOT(onReadySend()));
    userName = TeraflyCommunicator->userName;
	CURRENT_DATA_IS_SENT=false;
    numreceivedmessage=0;//for debug hl
    numsendmessage=0;
}

VR_MainWindow::~VR_MainWindow() {
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
}



void VR_MainWindow::onReadySend()
{

    qDebug()<<"VR_MainWindow::onReadySend()";
	if(!CollaborationSendPool.empty())
	{
		cout<<"CollaborationSendPool.size()"<<CollaborationSendPool.size()<<endl;
		QString send_MSG = *CollaborationSendPool.begin();
		CollaborationSendPool.erase(CollaborationSendPool.begin());
		if((send_MSG!="exit")&&(send_MSG!="quit"))
		{
            VR_Communicator->onReadySend("/seg:" + send_MSG);
		}

	}
	else
    {
		cout<<"CollaborationSendPool is empty";

	}
}


//void VR_MainWindow::onReadyRead()
void VR_MainWindow::TVProcess(QString line)

{
    QRegExp usersRex("^/users:(.*)$");
    QRegExp systemRex("^/system:(.*)$");
	QRegExp hmdposRex("^/hmdpos:(.*)$");
	QRegExp colorRex("^/color:(.*)$");
    QRegExp deletecurveRex("^/del_curve:(.*)__(.*)$");
    QRegExp markerRex("^/marker:(.*)__(.*)$");
    QRegExp delmarkerRex("^/del_marker:(.*)__(.*)$");
	QRegExp dragnodeRex("^/drag_node:(.*)$");
    QRegExp creatorRex("^/creator:(.*)__(.*)$");
    QRegExp messageRex("^/seg:(.*)__(.*)$");
    QRegExp retypeRex("^/retype:(.*)__(.*)$");

    line=line.trimmed();
        if (usersRex.indexIn(line) != -1) {
            QStringList users = usersRex.cap(1).split(",");

            foreach (QString user, users) {

                if(user==userName) continue;// skip itself

                bool findSameAgent=false;
                for(int i=0;i<VR_Communicator->Agents.size();i++)
                {
                    if(user==VR_Communicator->Agents.at(i).name)
                    {
                        findSameAgent=true;
                        break;
                    }
                }
                if(findSameAgent==false)
                {
                    Agent agent00={
                        user,
                        false,
                        21,
                        0

                    };
                    VR_Communicator->Agents.push_back(agent00);
                }
            }
        }
        else if (systemRex.indexIn(line) != -1) {

            QStringList sysMSGs = systemRex.cap(1).split(" ");
            if(sysMSGs.size()<2) return;

            QString user=sysMSGs.at(0);
            QString Action=sysMSGs.at(1);
            if((user!=userName)&&(Action=="joined"))
            {
                qDebug()<<"user: "<< user<<"joined";

                Agent agent00={
                    user,
                    false,
                    21,//colortypr
                    0, //POS
                };
                VR_Communicator->Agents.push_back(agent00);
            }
            else if((user!=userName)&&(Action=="left"))
            {
                qDebug()<<"user: "<< user<<"left";
                //the message is user ... left
                for(int i=0;i<VR_Communicator->Agents.size();i++)
                {
                    if(user == VR_Communicator->Agents.at(i).name)
                    {
                        //qDebug()<<"before erase "<<Agents.size();
                        VR_Communicator->Agents.erase(VR_Communicator->Agents.begin()+i);
                        i--;
                        //qDebug()<<"before erase "<<Agents.size();
                    }
                }
            }

        }
        else if(hmdposRex.indexIn(line) != -1) {
            QStringList hmdMSGs = hmdposRex.cap(1).split(" ");
            if(hmdMSGs.size()<17) return;

            QString user=hmdMSGs.at(0);
            if(user == userName) return;//the msg is the position of the current user,do nothing
            for(int i=0;i<VR_Communicator->Agents.size();i++)
            {
                if(user == VR_Communicator->Agents.at(i).name)// the msg is the position of user[i],update POS
                {
                    for(int j=0;j<16;j++)
                    {
                        VR_Communicator->Agents.at(i).position[j]=hmdMSGs.at(j+1).toFloat();

                    }
                    break;
                }
            }
        }
        else if(colorRex.indexIn(line) != -1) {
            QStringList clrMSGs = colorRex.cap(1).split(" ");

            if(clrMSGs.size()<2) return;
            QString user=clrMSGs.at(0);
            QString clrtype=clrMSGs.at(1);
            for(int i=0;i<VR_Communicator->Agents.size();i++)
            {
                if(VR_Communicator->Agents.at(i).name!=user) continue;
                //update agent color
                VR_Communicator->Agents.at(i).colorType=clrtype.toInt();
                qDebug()<<"user:"<<user<<" receievedColorTYPE="<<VR_Communicator->Agents.at(i).colorType;
                if(user == userName)
                    pMainApplication->SetupCurrentUserInformation(userName.toStdString(), VR_Communicator->Agents.at(i).colorType);
            }
        }
		else if (creatorRex.indexIn(line) != -1) {
			QString user = creatorRex.cap(1);
			QStringList markerMSGs = creatorRex.cap(2).split(" ");
            if (markerMSGs.size() < 7)
			{
                qDebug() << "size < 7";
                if(user==userName)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
				return;
			}

			float mx = markerMSGs.at(0).toFloat();
			float my = markerMSGs.at(1).toFloat();
			float mz = markerMSGs.at(2).toFloat();
			int resx, resy, resz,res;

			if (markerMSGs.size() > 3)
			{
				resx = markerMSGs.at(3).toFloat();
				resy = markerMSGs.at(4).toFloat();
				resz = markerMSGs.at(5).toFloat();
				res = markerMSGs.at(6).toInt();

			}

//            qDebug()<<"-----------creator reveive---"<<mx<<" "<<my<<" "<<mz<<" "<<resx<<" "<<resy<<" "<<resz<<" "<<res;
			if (pMainApplication)
			{

				pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
				XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);
                qDebug()<<"converreceivexyz"<<converreceivexyz.x<<" "<< converreceivexyz.y<<" "<< converreceivexyz.z;
				if (user == userName)
				{
					pMainApplication->READY_TO_SEND = false;
					CURRENT_DATA_IS_SENT = false;
					qDebug() << "get message CURRENT_DATA_IS_SENT=false;";
				}

                if (mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y || mz<VRVolumeStartPoint.z || mx>VRVolumeEndPoint.x || my>VRVolumeEndPoint.y || mz>VRVolumeEndPoint.z)
                {
                    qDebug() << "marker out of size";
                    VROutinfo.deletemarkerspos.push_back(QString("%1 %2 %3 %4").arg(mx).arg(my).arg(mz).arg(2));
                }else
                {
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, 2);
                }

                    pMainApplication->CollaborationCreatorPos = XYZ(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
                    VR_Communicator->CreatorMarkerPos = XYZ(mx, my, mz);
                    qDebug()<<"VR_Communicator->CreatorMarkerPos:"<<mx<<" "<<my<<" "<<mz;
                    VR_Communicator->CreatorMarkerRes = res;
			}

        }
        else if (deletecurveRex.indexIn(line) != -1) {
//            qDebug() << "deletecurve:"<<line;
            QString user = deletecurveRex.cap(1);
            QStringList delMSGs = deletecurveRex.cap(2).split("_",QString::SkipEmptyParts);
            delMSGs.pop_front();

            if(delMSGs.size()<1)
            {
                qDebug()<<"size < 2";
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                return;
            }


            for(int i=0;i<delMSGs.size();i++)
            {
                if(pMainApplication)
                {
                    QStringList xyz=delMSGs.at(i).split(" ",QString::SkipEmptyParts);
                    qDebug()<<"xyz list::"<<xyz;
                    if(xyz.size()<6) continue;
					float resx = xyz.at(3).toFloat();
					float resy = xyz.at(4).toFloat();
					float resz = xyz.at(5).toFloat();
					pMainApplication->collaborationTargetdelcurveRes.x = resx;
					pMainApplication->collaborationTargetdelcurveRes.y = resy;
					pMainApplication->collaborationTargetdelcurveRes.z = resz;

                    XYZ node=ConvertreceiveCoords(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat());
                    if(pMainApplication->DeleteSegment(node.x,node.y,node.z))
                    {
                        if(xyz.at(0).toFloat()<VRVolumeStartPoint.x ||xyz.at(1).toFloat()<VRVolumeStartPoint.y||xyz.at(2).toFloat()<VRVolumeStartPoint.z
                                ||xyz.at(0).toFloat()>VRVolumeEndPoint.x||xyz.at(1).toFloat()>VRVolumeEndPoint.y||xyz.at(2).toFloat()>VRVolumeEndPoint.z)
                        {
                            VROutinfo.deletedcurvespos.push_back(XYZ(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat()));
                            continue;
                        }
                    }
                }
             }
            if(user==userName||user.toInt()==userName.toInt()+1000)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }
        }
        else if (markerRex.indexIn(line) != -1) {
            QString user = markerRex.cap(1);
            QString markerPos=markerRex.cap(2).trimmed();
            QStringList markerMSGs =markerPos .split(" ");
            if(markerMSGs.size()<3)
            {
                qDebug()<<"size < 3";
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                return;
            }

            float mx = markerMSGs.at(0).toFloat();
            float my = markerMSGs.at(1).toFloat();
            float mz = markerMSGs.at(2).toFloat();
            int resx,resy,resz;

            if(markerMSGs.size()>3)
            {
                 resx = markerMSGs.at(3).toFloat();
                 resy = markerMSGs.at(4).toFloat();
                 resz = markerMSGs.at(5).toFloat();
            }
            int type=2;
            if(markerMSGs.size()==7)
                type=markerMSGs.at(6).toInt();

			if (pMainApplication)
			{
				pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
				XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);


                bool IsmarkerValid = false;
                IsmarkerValid = pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);

                if(!IsmarkerValid)
                {
                    if(mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y||mz<VRVolumeStartPoint.z|| mx>VRVolumeEndPoint.x||my>VRVolumeEndPoint.y||mz>VRVolumeEndPoint.z)
                    {
                        qDebug()<<"marker out of size";
                        VROutinfo.deletemarkerspos.push_back(markerPos);
                        return;
                    }else
                    {
                         pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, type);
                    }
                }

//                if(!IsmarkerValid&&(mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y||mz<VRVolumeStartPoint.z|| mx>VRVolumeEndPoint.x||my>VRVolumeEndPoint.y||mz>VRVolumeEndPoint.z))
//                {
//                    qDebug()<<"marker out of size";
//                    VROutinfo.deletemarkerspos.push_back(QString("%1 %2 %3 %4").arg(mx).arg(my).arg(mz).arg(3));
//                    return;
//                }else if(!IsmarkerValid)
//                {
//                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, 2);
//                }
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND = false;
                    CURRENT_DATA_IS_SENT = false;
                    qDebug() << "get message CURRENT_DATA_IS_SENT=false;";
                }
            }
       }
        else if (delmarkerRex.indexIn(line) != -1) {
            QStringList delmarkerPOS = delmarkerRex.cap(2).split(" ",QString::SkipEmptyParts);
                        QString user = delmarkerRex.cap(1);
            if(delmarkerPOS.size()<3)
            {
                qDebug()<<"size < 3";
                if(user==userName)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                return;
            }

            float mx = delmarkerPOS.at(0).toFloat();
            float my = delmarkerPOS.at(1).toFloat();
            float mz = delmarkerPOS.at(2).toFloat();
            int resx = delmarkerPOS.at(3).toFloat();
            int resy = delmarkerPOS.at(4).toFloat();
            int resz = delmarkerPOS.at(5).toFloat();
            qDebug()<<"user, "<<user<<"del marker: "<<mx<<" "<<my<<" "<<mz;
            pMainApplication->CollaborationTargetMarkerRes = XYZ(resx,resy,resz);
            XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
            if(user==userName)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }
//            int colortype=3;
//            for(int i=0;i<VR_Communicator->Agents.size();i++)
//            {
//                if(user == VR_Communicator->Agents.at(i).name)
//                {
//                    colortype=VR_Communicator->Agents.at(i).colorType;
//                    break;
//                }
//            }
            pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,3);
        }
        else if (dragnodeRex.indexIn(line) != -1) {
            QStringList dragnodePOS = dragnodeRex.cap(1).split(" ");
            if(dragnodePOS.size()<6)
            {
                qDebug()<<"error! size < 6";
                return;
            }
            QString user = dragnodePOS.at(0);
            int ntnum = dragnodePOS.at(1).toInt();
            int swcnum = dragnodePOS.at(2).toInt();
            float mx = dragnodePOS.at(3).toFloat();
            float my = dragnodePOS.at(4).toFloat();
            float mz = dragnodePOS.at(5).toFloat();
//            qDebug()<<"user, "<<user<<"drag node's num:"<<ntnum<<" "<<swcnum<<" new position: "<<mx<<" "<<my<<" "<<mz;
            XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
            if(user==userName)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
                pMainApplication->ClearCurrentNT();
            }
            pMainApplication->UpdateDragNodeinNTList(ntnum,swcnum,converreceivexyz.x,converreceivexyz.y,converreceivexyz.z);
        }
        else if (messageRex.indexIn(line) != -1) {

            QString user=messageRex.cap(1);
            QStringList MSGs = messageRex.cap(2).split("_",QString::SkipEmptyParts);//list of nodes: seg header_node 1_node 2.....
//            qDebug()<<MSGs[0];
            QString message;
            if(MSGs.size()<=1)
            {
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                return;
            }

            qDebug()<<"======================messageindex in TeraVr begin=================";
            QStringList REMSGS;
            for(int i=1;i<MSGs.size();i++)
                REMSGS.push_front(MSGs.at(i));
            REMSGS.push_front(MSGs.at(0));
//            qDebug()<<REMSGS;
            int colortype=21;
            for(int i=1;i<REMSGS.size();i++)
            {
                QString PointMSG=REMSGS.at(i);
                QStringList poingmsg=PointMSG.trimmed().split(" ");

                XYZ LocCoords=ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
                message+=QString(QString::number(i)+" "+poingmsg[1]+" "+QString::number(LocCoords.x)
                        +" "+QString::number(LocCoords.y)+" "+QString::number(LocCoords.z)+" "+poingmsg[5]+" ");
                if(i==1)
                {
                    message+=QString::number(-1);
                    colortype=poingmsg[1].toInt();
                    if(REMSGS[0].split(" ").at(0)=="TeraVR"||REMSGS[0].split(" ").at(0)=="TeraFly")
                    {
                        float mx = poingmsg[2].toFloat();
                        float my = poingmsg[3].toFloat();
                        float mz = poingmsg[4].toFloat();
                        int resx = REMSGS[0].split(" ").at(1).toInt();
                        int resy = REMSGS[0].split(" ").at(2).toInt();
                        int resz = REMSGS[0].split(" ").at(3).toInt();

                        qDebug()<<"mx="<<mx<<" my="<<my<<" mz="<<mz<<" resx="<<resx<<" resy="<<resy<<" resz="<<resz;
                        pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
                        XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);                      
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, 2);
//                        pMainApplication->CollaborationTargetMarkerRes = XYZ(MSGs[0].split(" ").at(1).toInt(), MSGs[0].split(" ").at(2).toInt(), MSGs[0].split(" ").at(3).toInt());
//                        XYZ  converreceivexyz = ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
//                        pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype,1);
                    }
                }
                else
                    message+=QString::number(i-1);

                if(i!=REMSGS.size()-1)
                {
                    message+=" ";

                }else
                {

                    if(REMSGS[0].split(" ").at(0)=="TeraVR"||REMSGS[0].split(" ").at(0)=="TeraFly")
                    {
                        pMainApplication->CollaborationTargetMarkerRes = XYZ(REMSGS[0].split(" ").at(1).toInt(), REMSGS[0].split(" ").at(2).toInt(), REMSGS[0].split(" ").at(3).toInt());
                        XYZ  converreceivexyz = ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
                        pMainApplication->RemoveMarkerandSurface2(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z,1);
                    }
                }

            }
            qDebug()<<"======================messageindex in TeraVr end=================";
            if(pMainApplication)
            {
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                pMainApplication->UpdateNTList(message,colortype);
            }
        }

        else if (retypeRex.indexIn(line)!=-1)
        {
            QString user = deletecurveRex.cap(1);
            QStringList delMSGs = deletecurveRex.cap(2).trimmed().split("_",QString::SkipEmptyParts);

            if(delMSGs.size()<1)
            {
                qDebug()<<"size < 2";
                if(user==userName||user.toInt()==userName.toInt()+1000)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                return;
            }
            for(int i=0;i<delMSGs.size();i++)
            {
                if(pMainApplication)
                {
                    QStringList xyz=delMSGs.at(i).split(" ",QString::SkipEmptyParts);
                    int type=xyz.at(3).toInt();
                    qDebug()<<"xyz list::"<<xyz;
                    if(xyz.size()<7) continue;
                    float resx = xyz.at(4).toFloat();
                    float resy = xyz.at(5).toFloat();
                    float resz = xyz.at(6).toFloat();
                    pMainApplication->collaborationTargetdelcurveRes.x = resx;
                    pMainApplication->collaborationTargetdelcurveRes.y = resy;
                    pMainApplication->collaborationTargetdelcurveRes.z = resz;

                    XYZ node=ConvertreceiveCoords(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat());
                    if(pMainApplication->retypeSegment(node.x,node.y,node.z,type))
                    {
                        if(xyz.at(0).toFloat()<VRVolumeStartPoint.x ||xyz.at(1).toFloat()<VRVolumeStartPoint.y||xyz.at(2).toFloat()<VRVolumeStartPoint.z
                                ||xyz.at(0).toFloat()>VRVolumeEndPoint.x||xyz.at(1).toFloat()>VRVolumeEndPoint.y||xyz.at(2).toFloat()>VRVolumeEndPoint.z)
                        {
                            VROutinfo.retypeMsgs.push_back(delMSGs.at(i));
                            continue;
                        }
                    }
                }
             }
            if(user==userName||user.toInt()==userName.toInt()+1000)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }
        }
}

int VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,
     bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator* TeraflyCommunicator,
                                XYZ* zoomPOS,XYZ *CreatorPos,XYZ MaxResolution) {

    pMainApplication = new CMainApplication(  0, 0 ,TeraflyCommunicator->CreatorMarkerPos);

	pMainApplication->mainwindow =pmain; 

	pMainApplication->isOnline = isLinkSuccess;
//    connect(pMainApplication,SIGNAL(undo()),VR_Communicator,SLOT(undo()));
    //pMainApplication->loadedNT.listNeuron.clear();
    //pMainApplication->loadedNT.hashNeuron.clear();
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
	//if(pMainApplication->loadedNTList->size()>0)
	//{
	//	pMainApplication->loadedNT.listNeuron = ntlist->at(0).listNeuron;
	//	pMainApplication->loadedNT.hashNeuron = ntlist->at(0).hashNeuron; 
	//}
	if(i4d->valid())
	{
		pMainApplication->img4d = i4d;
		pMainApplication->m_bHasImage4D=true;
	}
	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
//        disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
//        connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
		return 0;
	}
	SendVRconfigInfo();
	RunVRMainloop(zoomPOS);
	//pMainApplication->Shutdown();
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
        disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
        connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
		delete pMainApplication;
		pMainApplication=0;
		return _call_that_function;
}
void VR_MainWindow::SendHMDPosition()
{
	if(!pMainApplication) return;
	//get hmd position
	QString PositionStr=pMainApplication->getHMDPOSstr();

	//send hmd position
    VR_Communicator->onReadySend(QString("/hmdpos:" + PositionStr));
	//QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
	//cout<<"socket resindex"<<ResIndex<<endl;
	//qDebug()<<"QString resindex"<< QString("%1").arg(ResIndex);
    VR_Communicator->onReadySend(QString("/ResIndex:" + QString("%1").arg(ResIndex) ));
}
void VR_MainWindow::RunVRMainloop(XYZ* zoomPOS)
{
	qDebug()<<"get into RunMainloop";
	bool bQuit = false;
	int sendHMDPOScout = 0;


	while(!bQuit)
	{
	//update agents position if necessary
	if(VR_Communicator->Agents.size()>0)
		pMainApplication->SetupAgentModels(VR_Communicator->Agents);

	//handle one rendering loop, and handle user interaction
	bQuit=pMainApplication->HandleOneIteration();


	//send local data to server
    if((pMainApplication->undo==false)&&(pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
	//READY_TO_SEND is set to true by the "trigger button up" event;
	//client sends data to server (using onReadySend());
	//server sends the same data back to client;
	//READY_TO_SEND is set to false in onReadyRead();
	//CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
	{
        if(pMainApplication->m_modeGrip_R==m_drawMode)
		{
			qDebug() << pMainApplication->currentNT.listNeuron.size() << endl;
            QStringList waitsend=pMainApplication->NT2QString();
            QString send_string=waitsend[0]+" "+QString::number(VRVolumeCurrentRes.x)+" "
                    +QString::number(VRVolumeCurrentRes.y)+" "+QString::number(VRVolumeCurrentRes.z)+"_"+waitsend[1];
            if(send_string.size()!=0)
                CollaborationSendPool.emplace_back(send_string);
			pMainApplication->ClearCurrentNT();
            emit sendPoolHead();
			CURRENT_DATA_IS_SENT=true;
			qDebug()<<"CURRENT_DATA_IS_SENT=true;";
		}
		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
		{

//			qDebug() << "delname = " << pMainApplication->delName;
//			qDebug() << "delcurvePOS = " << pMainApplication->delcurvePOS;
			if (pMainApplication->SegNode_tobedeleted.x != 0 || pMainApplication->SegNode_tobedeleted.y != 0 || pMainApplication->SegNode_tobedeleted.z != 0)
				//socket->write(QString("/del_curve:" + pMainApplication->delName + "\n").toUtf8());
			//else
			{

				QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
				QString SegNode_tobedeleted = QString("%1 %2 %3").arg(pMainApplication->SegNode_tobedeleted.x).arg(pMainApplication->SegNode_tobedeleted.y).arg(pMainApplication->SegNode_tobedeleted.z);
				//QString delID = pMainApplication->FindNearestSegment(glm::vec3(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z));
				QString ConverteddelcurvePOS = ConvertsendCoords(SegNode_tobedeleted);
//				VR_Communicator->onReadySend(QString("/del_curve:" + ConverteddelcurvePOS + " " + QSCurrentRes));

                QStringList _undostringList=pMainApplication->UndoNT2QString();
//                qDebug()<<"_undostringList:"<<_undostringList;
                QString send_string=_undostringList[0]+" "+QString::number(VRVolumeCurrentRes.x)+" "
                        +QString::number(VRVolumeCurrentRes.y)+" "+QString::number(VRVolumeCurrentRes.z)+"_"+_undostringList[1];
//                qDebug()<<send_string;
                VR_Communicator->undo_delcure.push_back("/seg:"+send_string);
                VR_Communicator->onReadySend(QString("/del_curve:TeraVR_" + ConverteddelcurvePOS + " " + QSCurrentRes));
				CURRENT_DATA_IS_SENT=true;
			}

			else if(pMainApplication->delName=="")
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
		}
		else if(pMainApplication->m_modeGrip_R==m_markMode)
		{
            if(pMainApplication->markerPOS!="")
            {
                QString ConvertedmarkerPOS = ConvertsendCoords(pMainApplication->markerPOS);
                QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
                VR_Communicator->onReadySend(QString("/marker:" + ConvertedmarkerPOS +" "+QSCurrentRes ));
                CURRENT_DATA_IS_SENT=true;
            }
		}

		else if(pMainApplication->m_modeGrip_R==m_dragMode)
		{
			QString ConverteddragnodePOS = ConvertsendCoords(pMainApplication->dragnodePOS);
            VR_Communicator->onReadySend(QString("/drag_node:" + ConverteddragnodePOS ));
			CURRENT_DATA_IS_SENT=true;
		}

		else if (pMainApplication->m_modeGrip_R == _MovetoCreator)
		{
			QString ConvertedmarkerPOS = ConvertsendCoords(pMainApplication->markerPOS);
			QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
			QString QCmainResIndex = QString("%1").arg(pMainApplication->CmainResIndex);
			VR_Communicator->onReadySend(QString("/creator:" + ConvertedmarkerPOS + " " + QSCurrentRes + " " + QCmainResIndex));
            qDebug()<<QString("/creator:" + ConvertedmarkerPOS + " " + QSCurrentRes + " " + QCmainResIndex);
			CURRENT_DATA_IS_SENT = true;
        }


		//if(pMainApplication->READY_TO_SEND==true)
		//	CURRENT_DATA_IS_SENT=true;
		
	}

    //left hand undo
    if((pMainApplication->undo==true)&&(pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
    {
        qDebug()<<"---------undo TV------------";
        VR_Communicator->undo();
        CURRENT_DATA_IS_SENT=true;
        pMainApplication->undo=false;
    }

	sendHMDPOScout++;
	//if(sendHMDPOScout>30)
	//{
	//	SendHMDPosition();
	//	socket->write(QString("/ask:message \n").toUtf8());
	//	sendHMDPOScout = 0;}
	//}
	if(sendHMDPOScout%20==0)
	{
        VR_Communicator->onReadySend(QString("/ask:message"));
	}
	if(sendHMDPOScout%60==0)
	{
		SendHMDPosition();
		sendHMDPOScout = 0;
	}
	// switch (sendHMDPOScout/20)
	// {
	// case 1:
	// 	socket->write(QString("/ask:message \n").toUtf8());
	// 	break;
	// case 3:
	// 	SendHMDPosition();
	// 	sendHMDPOScout = 0;
	// 	break;
	// default:
	// 	break;
	// }
	}
	//QTimer::singleShot(20, this, SLOT(RunVRMainloop()));
	return ;
}

//-----------------------------------------------------------------------------
// Purpose: for standalone VR.
//-----------------------------------------------------------------------------
int startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain, XYZ* zoomPOS)
// bool startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain)
{
	VR_Window * pmainwindow = new VR_Window();
	pmainwindow->show();

	//------------------------------origin vr function-------------------------------------//
	//CMainApplication *pMainApplication = new CMainApplication( 0, 0 );
	////pMainApplication->setnetworkmodefalse();//->NetworkModeOn=false;
 //   pMainApplication->mainwindow = pmain;
 //   pMainApplication->isOnline = false;


	//if(ntlist != NULL)
	//{
	//	if((ntlist->size()==1)&&(ntlist->at(0).name.isEmpty()))
	//	{
	//		// means there is only a reloaded annotation in terafly
	//		// we rename it as vaa3d_traced_neuron
	//		qDebug()<<"means this is terafly special condition.do something";
	//		NeuronTree newS;
	//		newS.color = XYZW(0,0,255,255);
	//		newS = ntlist->at(0);
	//		newS.n = 1;
	//		newS.on = true;
	//		newS.name = "vaa3d_traced_neuron";
	//		newS.file = "vaa3d_traced_neuron";
	//		pMainApplication->editableLoadedNTL.append(newS); 
	//	}
	//	else
	//	{
	//		for(int i=0;i<ntlist->size();i++)
	//		{
	//			if((ntlist->at(i).name == "vaa3d_traced_neuron")&&(ntlist->at(i).file == "vaa3d_traced_neuron"))
	//			{
	//				// means there is a NT named "vaa3d_traced_neuron", we only need to edit this NT.
	//				pMainApplication->editableLoadedNTL.append(ntlist->at(i));
	//			}
	//			else if (!ntlist->at(0).name.isEmpty())
	//			{
	//				// means it is a loaded Neuron in 3D View,currently we do not allow to edit this neuron in VR
	//				pMainApplication->nonEditableLoadedNTL.append(ntlist->at(i));
	//			}
	//			// else if (ntlist->at(0).name.isEmpty())
	//			// means it is an reloaded annotation in terafly, currently we do not show this neuron in VR
	//		}
	//	}
	//}
 //   pMainApplication->loadedNTList = ntlist;
	//
	//if(i4d->valid())
	//{
	//	pMainApplication->img4d = i4d;
	//	pMainApplication->m_bHasImage4D=true;
	//}
	//if (!pMainApplication->BInit())
	//{
	//	pMainApplication->Shutdown();
	//	return 0;
	//}
	//pMainApplication->SetupCurrentUserInformation("local user", 13);

	//pMainApplication->RunMainLoop();

	//pMainApplication->Shutdown();

	//// bool _call_that_plugin = pMainApplication->_call_assemble_plugin;
	//int _call_that_function = pMainApplication->postVRFunctionCallMode;
	//zoomPOS->x = pMainApplication->teraflyPOS.x;
	//zoomPOS->y = pMainApplication->teraflyPOS.y;
	//zoomPOS->z = pMainApplication->teraflyPOS.z;
	//delete pMainApplication;
	//pMainApplication = NULL;

	//// return _call_that_plugin;
	//return _call_that_function;

	return 0;
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

	
	pMainApplication->CollaborationMaxResolution = CollaborationMaxResolution;
	pMainApplication->CollaborationCurrentRes = VRVolumeCurrentRes;

}

QString VR_MainWindow::ConvertsendCoords(QString coords)
{
	float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
	float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
	float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
	x+=(VRVolumeStartPoint.x-1);
	y+=(VRVolumeStartPoint.y-1);
	z+=(VRVolumeStartPoint.z-1);
	x*=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
	y*=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
	z*=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
	return QString("%1 %2 %3").arg(x).arg(y).arg(z);
}

void VR_MainWindow::SendVRconfigInfo()
{
	float globalscale = pMainApplication->GetGlobalScale();
	QString QSglobalscale = QString("%1").arg(globalscale); 
    VR_Communicator->onReadySend(QString("/scale:" +  QSglobalscale ));
}

XYZ VR_MainWindow:: ConvertreceiveCoords(float x,float y,float z)
{
	//QString str1 = coords.section(' ',0, 0);  // str == "bin/myapp"
	//QString str2 = coords.section(' ',1, 1);  // str == "bin/myapp"
	//QString str3 = coords.section(' ',2, 2);  // str == "bin/myapp"

	float dividex = VRvolumeMaxRes.x/VRVolumeCurrentRes.x;

	float dividey = VRvolumeMaxRes.y/VRVolumeCurrentRes.y;
	float dividez = VRvolumeMaxRes.z/VRVolumeCurrentRes.z;
	x/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
	y/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
	z/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
	x-=(VRVolumeStartPoint.x-1);
	y-=(VRVolumeStartPoint.y-1);
	z-=(VRVolumeStartPoint.z-1);
	return XYZ(x,y,z);
}


VR_Window::VR_Window(QWidget *parent)
	: QGLWidget(parent)
{
	resizeGL(800,600);
	initializeGL();
}

VR_Window::~VR_Window()
{

}

void VR_Window::initializeGL()
{
	glClearColor(0.0, 0.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);


}

void VR_Window::resizeGL(int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);


}
void VR_Window::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void VR_Window::runcomputeshader_occu()
{
	glUseProgram(g_programOCC);
	//GL_ERROR();
	glBindImageTexture(0, g_occupancymap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	GLint transferFuncLoc = glGetUniformLocation(g_programOCC, "transfer_function");
	if (transferFuncLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, g_tffTexObj);
		glUniform1i(transferFuncLoc, 1);
	}
	else
	{
		std::cout << "TransferFunc"
			<< "is not bind to the uniform"
			<< endl;
	}

	GLint volumeLoc = glGetUniformLocation(g_programOCC, "volume");
	if (volumeLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, g_volTexObj);
		//glBindImageTexture(1, g_volTexObj, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
		glUniform1i(volumeLoc, 2);
	}
	else
	{
		std::cout << "Volume"
			<< "is not bind to the uniform"
			<< endl;
	}

	glDispatchCompute(10, 10, 4);////////////////////////////////?
	//GL_ERROR();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//GL_ERROR();
	glUseProgram(0);
}

void VR_Window::runcomputeshader_dis()
{
	//1ST

	glUseProgram(g_programDIS);
	//glUniform1i(stageLocation, t);
	int stageLocation0 = glGetUniformLocation(g_programDIS, "stage");
	int t0 = 0;
	if (stageLocation0 >= 0) {

		glUniform1i(stageLocation0, t0);
	}
	else
	{
		std::cout << "stage"
			<< "is not bind to the uniform"
			<< endl;
	}
	glBindImageTexture(0, g_distancemap, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	glBindImageTexture(1, g_occupancymap, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	glDispatchCompute(10, 10, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	//GL_ERROR();
	//2ST
	glUseProgram(g_programDIS);
	int stageLocation1 = glGetUniformLocation(g_programDIS, "stage");
	int t1 = 1;
	if (stageLocation1 >= 0) {

		glUniform1i(stageLocation1, t1);
	}
	else
	{
		std::cout << "stage"
			<< "is not bind to the uniform"
			<< endl;
	}

	//GL_ERROR();
	glBindImageTexture(0, g_distancemap, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(1, g_occupancymap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(10, 10, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	//3ST
	glUseProgram(g_programDIS);
	int stageLocation2 = glGetUniformLocation(g_programDIS, "stage");
	int t2 = 2;
	if (stageLocation2 >= 0) {

		glUniform1i(stageLocation2, t2);
	}
	else
	{
		std::cout << "stage"
			<< "is not bind to the uniform"
			<< endl;
	}
	//GL_ERROR();
	glBindImageTexture(0, g_distancemap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glBindImageTexture(1, g_occupancymap, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	glDispatchCompute(10, 10, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	//GL_ERROR();
}

void VR_Window::initest()
{
	const char *vertexShaderSource = "#version 430 core\n"
		"\n"
		"in vec3 vert;\n"
		"out vec3 vert_col;\n"
		"uniform mat4 MVP;"
		"\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = MVP*vec4(vert.x,vert.y,vert.z,1.0);\n"
		//"    gl_Position =vec4(vert.x,vert.y,vert.z,1.0);\n"
		"	 vert_col=vec3(vert.x,vert.y,vert.z);"
		"}\n";
	const char *fragmentShaderSource = "#version 430 core\n"
		"\n"
		"uniform sampler3D g_occupancymap;\n"
		"out vec4 color;\n"
		"\n"
		"in vec3 vert_col;\n"
		"void main(void)\n"
		"{\n"
		"color=texture(g_occupancymap,vert_col);"
		//"color=vec4(1,0,0,0);"
		"}\n";
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog <<
			std::endl;
	}
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR:SHADER::PROGRAM::LINKING_FAILED\n" << infoLog <<
			std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,

		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f
	};




	//vao,vbo登场！
	unsigned int VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//先绑定VAO，然后绑定VBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//设置顶点属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);//???
	glBindVertexArray(0);//????
}

void VR_Window::linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
	const GLsizei maxCount = 2;
	GLsizei count;
	GLuint shaders[maxCount];
	glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);
	// cout << "get VertHandle: " << shaders[0] << endl;
	// cout << "get FragHandle: " << shaders[1] << endl;
	//GL_ERROR();
	for (int i = 0; i < count; i++) {
		glDetachShader(shaderPgm, shaders[i]);
	}
	// Bind index 0 to the shader input variable "VerPos"
	glBindAttribLocation(shaderPgm, 0, "VerPos");
	// Bind index 1 to the shader input variable "VerClr"
	glBindAttribLocation(shaderPgm, 1, "VerClr");
	//GL_ERROR();
	glAttachShader(shaderPgm, newVertHandle);
	glAttachShader(shaderPgm, newFragHandle);
	//GL_ERROR();
	glLinkProgram(shaderPgm);
	if (GL_FALSE == checkShaderLinkStatus(shaderPgm))
	{
		cerr << "Failed to relink shader program!" << endl;
		exit(EXIT_FAILURE);
	}
	//GL_ERROR();
}

void VR_Window::render(GLenum cullFace,GLuint g_programid)
{
	//GL_ERROR();
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//  transform the box
	glm::mat4 projection = glm::perspective(60.0f, (GLfloat)g_winWidth / g_winHeight, 0.1f, 400.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model = mat4(1.0f);
	model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	// to make the "head256.raw" i.e. the volume data stand up.
	model *= glm::rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
	model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));
	// notice the multiplication order: reverse order of transform
	glm::mat4 mvp = projection * view * model;
	GLuint mvpIdx = glGetUniformLocation(g_programid, "MVP");
	if (mvpIdx >= 0)
	{
		glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
	}
	else
	{
		cerr << "can't get the MVP" << endl;
	}
	//GL_ERROR();
	drawBox(cullFace);
	//GL_ERROR();
	// glutWireTeapot(0.5);
}


void VR_Window::initShader()
{
	// vertex shader object for first pass
	//g_bfVertHandle = initShaderObj("shader/backface.vert", GL_VERTEX_SHADER);
	//// fragment shader object for first pass
	//g_bfFragHandle = initShaderObj("shader/backface.frag", GL_FRAGMENT_SHADER);
	//// vertex shader object for second pass
	//g_rcVertHandle = initShaderObj("shader/raycasting.vert", GL_VERTEX_SHADER);
	//// fragment shader object for second pass
	//g_rcFragHandle = initShaderObj("shader/renderring_Acc.frag", GL_FRAGMENT_SHADER);
	// create the shader program , use it in an appropriate time
	backfaceShader = new Shader(string("shader/backface.vert").c_str(), string("shader/backface.frag").c_str());
	raycastingShader = new Shader(string("shader/raycasting.vert").c_str(), string("shader/renderring_Acc.frag").c_str());
	//g_programHandle = createShaderPgm();
	// 获得由着色器编译器分配的索引(可选)
	//compute occupancy_map
	//GL_ERROR();
	//g_compute_occ = initShaderObj("shader/occupancy_map.comp", GL_COMPUTE_SHADER);
	g_programOCC = CompileGLShader("occupancy_map", "", GL_COMPUTE_SHADER);
	//g_programOCC = createShaderPgm();
	////GL_ERROR();
	//int success;
	//char infoLog[512];
	//glAttachShader(g_programOCC, g_compute_occ);
	//glLinkProgram(g_programOCC);
	//glGetProgramiv(g_programOCC, GL_LINK_STATUS, &success);
	//if (!success) {
	//	glGetProgramInfoLog(g_compute_occ, 512, NULL, infoLog);
	//	std::cout << "ERROR::OCC::COMPUTE::PROGRAM::LINKING_FAILED\n" << infoLog <<
	//		std::endl;
	//}

	//compute distance_map
	g_programDIS = CompileGLShader("distance_map", "", GL_COMPUTE_SHADER);//liqi func below is encapsulated into CompileGLShader
	//g_compute_dis = initShaderObj("shader/distance_map.comp", GL_COMPUTE_SHADER);
	/*g_programDIS = createShaderPgm();
	glAttachShader(g_programDIS, g_compute_dis);
	glLinkProgram(g_programDIS);
	glGetProgramiv(g_programDIS, GL_LINK_STATUS, &success);
	if (!success) {
	glGetProgramInfoLog(g_compute_occ, 512, NULL, infoLog);
	std::cout << "ERROR::DIS::COMPUTE::PROGRAM::LINKING_FAILED\n" << infoLog <<
	std::endl;
	}*/
}

GLuint VR_Window::initOccupancyTex()
{
	GLuint occu;
	glGenTextures(1, &occu);
	glBindTexture(GL_TEXTURE_3D, occu);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, 50, 50, 32);
	glBindTexture(GL_TEXTURE_3D, 0);
	//glBindImageTexture(2, g_occupancymap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	return occu;
}

void VR_Window::initVBO()
{
	GLfloat vertices[24] = {
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 1.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 1.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 1.0
	};
	// draw the six faces of the boundbox by drawwing triangles
	// draw it contra-clockwise
	// front: 1 5 7 3
	// back: 0 2 6 4
	// left：0 1 3 2
	// right:7 5 4 6    
	// up: 2 3 7 6
	// down: 1 0 4 5
	GLuint indices[36] = {
		1, 5, 7,
		7, 3, 1,
		0, 2, 6,
		6, 4, 0,
		0, 1, 3,
		3, 2, 0,
		7, 5, 4,
		4, 6, 7,
		2, 3, 7,
		7, 6, 2,
		1, 0, 4,
		4, 5, 1
	};
	GLuint gbo[2];

	glGenBuffers(2, gbo);
	GLuint vertexdat = gbo[0];
	GLuint veridxdat = gbo[1];
	glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	// used in glDrawElement()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	// vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0); // for vertexloc
	glEnableVertexAttribArray(1); // for vertexcol

	// the vertex location is the same as the vertex color
	glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat*)NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat*)NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
	// glBindVertexArray(0);
	g_vao = vao;
}

void VR_Window::drawBox(GLenum glFaces)
{
	//glBindImageTexture(0, g_distancemap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI);
	glEnable(GL_CULL_FACE);
	glCullFace(glFaces);
	glBindVertexArray(g_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint*)NULL);
	glDisable(GL_CULL_FACE);
}

GLboolean VR_Window::compileCheck(GLuint shader)
{

	GLint err;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
	if (GL_FALSE == err)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = (char*)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			cerr << "Shader log: " << log << endl;
			free(log);
		}
	}
	return err;
}
//
//GLuint VR_Window::initShaderObj(const char* srcfile, GLenum shaderType)
//{
//	ifstream inFile(srcfile, ifstream::in);
//	// use assert?
//	if (!inFile)
//	{
//		cerr << "Error openning file: " << srcfile << endl;
//		exit(EXIT_FAILURE);
//	}
//
//	const int MAX_CNT = 10000;
//	GLchar* shaderCode = (GLchar*)calloc(MAX_CNT, sizeof(GLchar));
//	inFile.read(shaderCode, MAX_CNT);
//	if (inFile.eof())
//	{
//		size_t bytecnt = inFile.gcount();
//		*(shaderCode + bytecnt) = '\0';
//	}
//	else if (inFile.fail())
//	{
//		std::cout << srcfile << "read failed " << endl;
//	}
//	else
//	{
//		std::cout << srcfile << "is too large" << endl;
//	}
//	// create the shader Object
//	GLuint shader = glCreateShader(shaderType);
//	if (0 == shader)
//	{
//		cerr << "Error creating vertex shader." << endl;
//	}
//	// cout << shaderCode << endl;
//	// cout << endl;
//	const GLchar* codeArray[] = { shaderCode };
//	glShaderSource(shader, 1, codeArray, NULL);
//	free(shaderCode);
//
//	// compile the shader
//	glCompileShader(shader);
//	if (GL_FALSE == compileCheck(shader))
//	{
//		cerr << "shader compilation failed" << endl;
//	}
//	return shader;
//}
static bool g_bbPrintf = true;
void vr_windowdprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bbPrintf)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}
GLuint VR_Window::CompileGLShader(const char *pchShaderName, const char *p_shader, GLenum shaderType)
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(shaderType);
	glShaderSource(nSceneVertexShader, 1, &p_shader, NULL);
	glCompileShader(nSceneVertexShader);

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		vr_windowdprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneVertexShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneVertexShader);
	glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached
	glLinkProgram(unProgramID);
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(unProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		vr_windowdprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram(unProgramID);
		return 0;
	}

	glUseProgram(unProgramID);
	glUseProgram(0);

	return unProgramID;
}

GLint VR_Window::checkShaderLinkStatus(GLuint pgmHandle)
{
	GLint status;
	glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
	if (GL_FALSE == status)
	{
		GLint logLen;
		glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			GLchar* log = (GLchar*)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pgmHandle, logLen, &written, log);
			cerr << "Program log: " << log << endl;
		}
	}
	return status;
}

GLuint VR_Window::createShaderPgm()
{
	// Create the shader program
	GLuint programHandle = glCreateProgram();
	if (0 == programHandle)
	{
		cerr << "Error create shader program" << endl;
		exit(EXIT_FAILURE);
	}
	return programHandle;
}

GLuint VR_Window::initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
	GLuint backFace2DTex;
	glGenTextures(1, &backFace2DTex);
	glBindTexture(GL_TEXTURE_2D, backFace2DTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	return backFace2DTex;
}

void VR_Window::checkFramebufferStatus()
{
	GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (complete != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "framebuffer is not complete" << endl;
		exit(EXIT_FAILURE);
	}
}

void VR_Window::initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
	// create a depth buffer for our framebuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

	// attach the texture and the depth buffer to the framebuffer
	glGenFramebuffers(1, &g_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	checkFramebufferStatus();
	glEnable(GL_DEPTH_TEST);
}

void VR_Window::rcSetUinforms(GLuint g_programid)
{
	// setting uniforms such as
	// ScreenSize 
	// StepSize
	// TransferFunc
	// ExitPoints i.e. the backface, the backface hold the ExitPoints of ray casting
	// VolumeTex the texture that hold the volume data i.e. head256.raw
	GLint screenSizeLoc = glGetUniformLocation(g_programid, "ScreenSize");
	if (screenSizeLoc >= 0)
	{
		glUniform2f(screenSizeLoc, (float)g_winWidth, (float)g_winHeight);
	}
	else
	{
		std::cout << "ScreenSize"
			<< "is not bind to the uniform"
			<< endl;
	}
	GLint stepSizeLoc = glGetUniformLocation(g_programid, "StepSize");
	//GL_ERROR();
	if (stepSizeLoc >= 0)
	{
		glUniform1f(stepSizeLoc, g_stepSize);
	}
	else
	{
		std::cout << "StepSize"
			<< "is not bind to the uniform"
			<< endl;
	}
	//GL_ERROR();
	//GLint transferFuncLoc = -1;
	GLint transferFuncLoc = glGetUniformLocation(g_programid, "TransferFunc");
	if (transferFuncLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, g_tffTexObj);
		glUniform1i(transferFuncLoc, 2);
	}
	else
	{
		std::cout << "TransferFunc"
			<< "is not bind to the uniform"
			<< endl;
	}
	//GL_ERROR();
	//GLint backFaceLoc = -1;
	GLint backFaceLoc = glGetUniformLocation(g_programid, "ExitPoints");
	if (backFaceLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
		glUniform1i(backFaceLoc, 0);
	}
	else
	{
		std::cout << "ExitPoints"
			<< "is not bind to the uniform"
			<< endl;
	}
	//GL_ERROR();
	GLint volumeLoc = glGetUniformLocation(g_programid, "VolumeTex");
	if (volumeLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, g_volTexObj);
		//glBindImageTexture(1, g_volTexObj, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
		glUniform1i(volumeLoc, 1);
	}
	else
	{
		std::cout << "VolumeTex"
			<< "is not bind to the uniform"
			<< endl;
	}

}