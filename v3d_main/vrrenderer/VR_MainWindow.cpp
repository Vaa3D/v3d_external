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
	QRegExp creatorRex("^/creator:(.*)$");
    QRegExp messageRex("^/seg:(.*)__(.*)$");

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
			if (markerMSGs.size() < 3)
			{
				qDebug() << "size < 3";
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
			if (pMainApplication)
			{

				pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
				XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);
				if (user == userName)
				{
					pMainApplication->READY_TO_SEND = false;
					CURRENT_DATA_IS_SENT = false;
					qDebug() << "get message CURRENT_DATA_IS_SENT=false;";
				}
				int colortype = 3;
				for (int i = 0; i < VR_Communicator->Agents.size(); i++)
				{
					if (user == VR_Communicator->Agents.at(i).name)
					{
						colortype = VR_Communicator->Agents.at(i).colorType;
						break;
					}
				}
				if (mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y || mz<VRVolumeStartPoint.z || mx>VRVolumeEndPoint.x || my>VRVolumeEndPoint.y || mz>VRVolumeEndPoint.z)
				{
					qDebug() << "marker out of size";
					VROutinfo.deletemarkerspos.push_back(QString("%1 %2 %3 %4").arg(mx).arg(my).arg(mz).arg(colortype));
					return;
				}
				bool IsmarkerValid = false;
				IsmarkerValid = pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
				cout << "IsmarkerValid is " << IsmarkerValid << endl;
				if (!IsmarkerValid)
				{

//                    qDebug()<<"flag _ :";
//                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, 3);
//					pMainApplication->collaboration_creator_res = res;
//					pMainApplication->CollaborationCreatorPos = XYZ(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
					

                        VR_Communicator->CreatorMarkerPos = XYZ(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
                        VR_Communicator->CreatorMarkerRes = res;
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, colortype);

				}
			}


            //qDebug()<<"get creator message";
            //QStringList creatorMSGs = creatorRex.cap(1).split(" ");
            //QString user=creatorMSGs.at(0);
            //QString creator_Res = creatorMSGs.at(1);
            //for(int i=0;i<VR_Communicator->Agents.size();i++)
            //{
            //    qDebug()<<"creator name is "<<user;
            //    if(VR_Communicator->Agents.at(i).name!=user) continue;
            //    pMainApplication->collaboration_creator_name = user;
            //    pMainApplication->collaboration_creator_res = creator_Res.toInt();
            //    qDebug()<<"user:"<<user<<" receievedCreator"<<pMainApplication->collaboration_creator_name;
            //    qDebug()<<"user:"<<user<<" receievedCreator res"<<pMainApplication->collaboration_creator_res;
            //}

        }
        else if (deletecurveRex.indexIn(line) != -1) {
            qDebug() << "deletecurve:"<<line;
            QString user = deletecurveRex.cap(1);
            QStringList delMSGs = deletecurveRex.cap(2).split("_",QString::SkipEmptyParts);
            if(delMSGs.size()<1)
            {
                qDebug()<<"size < 2";
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
					cout << "pMainApplication->collaborationTargetdelcurveRes.x" << pMainApplication->collaborationTargetdelcurveRes.x << endl;
					cout << "pMainApplication->collaborationTargetdelcurveRes.y" << pMainApplication->collaborationTargetdelcurveRes.y << endl;
					cout << "pMainApplication->collaborationTargetdelcurveRes.z" << pMainApplication->collaborationTargetdelcurveRes.z << endl;
                    if(xyz.at(0).toFloat()<VRVolumeStartPoint.x ||xyz.at(1).toFloat()<VRVolumeStartPoint.y||xyz.at(2).toFloat()<VRVolumeStartPoint.z
                            ||xyz.at(0).toFloat()>VRVolumeEndPoint.x||xyz.at(1).toFloat()>VRVolumeEndPoint.y||xyz.at(2).toFloat()>VRVolumeEndPoint.z
                    )
                    {
                        qDebug()<<"====================out =====================";
                        VROutinfo.deletedcurvespos.push_back(XYZ(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat()));
                        continue;
                    }

                    XYZ node=ConvertreceiveCoords(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat());
                    qDebug()<<"delcurve:==="<<node.x<<" "<<node.y<<" "<<node.z;
                    pMainApplication->DeleteSegment(node.x,node.y,node.z);
                }
             }

                    qDebug()<<".................................";
            if(user==userName)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }
        }
        else if (markerRex.indexIn(line) != -1) {
            QString user = markerRex.cap(1);
            QStringList markerMSGs = markerRex.cap(2).split(" ");
            if(markerMSGs.size()<3)
            {
                qDebug()<<"size < 3";
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
			if (pMainApplication)
			{
				pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
				XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);
				if (user == userName)
				{
					pMainApplication->READY_TO_SEND = false;
					CURRENT_DATA_IS_SENT = false;
					qDebug() << "get message CURRENT_DATA_IS_SENT=false;";
				}
				int colortype = 3;
				for (int i = 0; i < VR_Communicator->Agents.size(); i++)
				{
					if (user == VR_Communicator->Agents.at(i).name)
					{
						colortype = VR_Communicator->Agents.at(i).colorType;
						break;
					}
				}
                if(mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y||mz<VRVolumeStartPoint.z|| mx>VRVolumeEndPoint.x||my>VRVolumeEndPoint.y||mz>VRVolumeEndPoint.z)
                {
                    qDebug()<<"marker out of size";
                    VROutinfo.deletemarkerspos.push_back(QString("%1 %2 %3 %4").arg(mx).arg(my).arg(mz).arg(3));
                    return;
                }
				bool IsmarkerValid = false;
				IsmarkerValid = pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
				cout << "IsmarkerValid is " << IsmarkerValid << endl;
				if (!IsmarkerValid)
				{
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, 3);
				}
			}
        }
        else if (delmarkerRex.indexIn(line) != -1) {
            QStringList delmarkerPOS = delmarkerRex.cap(2).split(" ",QString::SkipEmptyParts);
            if(delmarkerPOS.size()<3)
            {
                qDebug()<<"size < 3";
                return;
            }
            QString user = delmarkerRex.cap(1);
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
            int colortype=3;
            for(int i=0;i<VR_Communicator->Agents.size();i++)
            {
                if(user == VR_Communicator->Agents.at(i).name)
                {
                    colortype=VR_Communicator->Agents.at(i).colorType;
                    break;
                }
            }
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
            qDebug()<<"user, "<<user<<"drag node's num:"<<ntnum<<" "<<swcnum<<" new position: "<<mx<<" "<<my<<" "<<mz;
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
            qDebug()<<MSGs[0];
            QString message;
            if(MSGs.size()<=1) return;

            qDebug()<<"======================messageindex in TeraVr begin=================";

            int colortype=21;
            for(int i=1;i<MSGs.size();i++)
            {
                QString PointMSG=MSGs.at(i);
                QStringList poingmsg=PointMSG.trimmed().split(" ");

                XYZ LocCoords=ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
                message+=QString(QString::number(i)+" "+poingmsg[1]+" "+QString::number(LocCoords.x)
                        +" "+QString::number(LocCoords.y)+" "+QString::number(LocCoords.z)+" "+poingmsg[5]+" ");
                if(i==1)
                {
                    message+=QString::number(-1);
                    colortype=poingmsg[1].toInt();
                    if(MSGs[0].split(" ").at(0)=="TeraVR"||MSGs[0].split(" ").at(0)=="TeraFly")
                    {
                        float mx = poingmsg[2].toFloat();
                        float my = poingmsg[3].toFloat();
                        float mz = poingmsg[4].toFloat();
                        int resx = MSGs[0].split(" ").at(1).toInt();
                        int resy = MSGs[0].split(" ").at(2).toInt();
                        int resz = MSGs[0].split(" ").at(3).toInt();

                        qDebug()<<"mx="<<mx<<" my="<<my<<" mz="<<mz<<" resx="<<resx<<" resy="<<resy<<" resz="<<resz;
                        pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
                        XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);
                        pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z,1);
//                        pMainApplication->CollaborationTargetMarkerRes = XYZ(MSGs[0].split(" ").at(1).toInt(), MSGs[0].split(" ").at(2).toInt(), MSGs[0].split(" ").at(3).toInt());
//                        XYZ  converreceivexyz = ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
//                        pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype,1);
                    }
                }
                else
                    message+=QString::number(i-1);

                if(i!=MSGs.size()-1)
                {
                    message+=" ";

                }else
                {

                    if(MSGs[0].split(" ").at(0)=="TeraVR"||MSGs[0].split(" ").at(0)=="TeraFly")
                    {
                        pMainApplication->CollaborationTargetMarkerRes = XYZ(MSGs[0].split(" ").at(1).toInt(), MSGs[0].split(" ").at(2).toInt(), MSGs[0].split(" ").at(3).toInt());
                        XYZ  converreceivexyz = ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, colortype);
                    }
                }

            }
            qDebug()<<"======================messageindex in TeraVr end=================";
            if(pMainApplication)
            {
                if(user==userName)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                pMainApplication->UpdateNTList(message,colortype);
            }
        }
}

int VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,
     bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator* TeraflyCommunicator,
                                XYZ* zoomPOS,XYZ *CreatorPos,XYZ MaxResolution) {

	pMainApplication = new CMainApplication(  0, 0 );

	pMainApplication->mainwindow =pmain; 

	pMainApplication->isOnline = isLinkSuccess;
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
	if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
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

			qDebug() << "delname = " << pMainApplication->delName;
			qDebug() << "delcurvePOS = " << pMainApplication->delcurvePOS;
			if (pMainApplication->SegNode_tobedeleted.x != 0 || pMainApplication->SegNode_tobedeleted.y != 0 || pMainApplication->SegNode_tobedeleted.z != 0)
				//socket->write(QString("/del_curve:" + pMainApplication->delName + "\n").toUtf8());
			//else
			{

				QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
				QString SegNode_tobedeleted = QString("%1 %2 %3").arg(pMainApplication->SegNode_tobedeleted.x).arg(pMainApplication->SegNode_tobedeleted.y).arg(pMainApplication->SegNode_tobedeleted.z);
				//QString delID = pMainApplication->FindNearestSegment(glm::vec3(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z));
				QString ConverteddelcurvePOS = ConvertsendCoords(SegNode_tobedeleted);
				VR_Communicator->onReadySend(QString("/del_curve:" + ConverteddelcurvePOS + " " + QSCurrentRes));
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
            QString ConvertedmarkerPOS = ConvertsendCoords(pMainApplication->markerPOS);
			QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
            VR_Communicator->onReadySend(QString("/marker:" + ConvertedmarkerPOS +" "+QSCurrentRes ));
			CURRENT_DATA_IS_SENT=true;
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
			CURRENT_DATA_IS_SENT = true;
		}
		//if(pMainApplication->READY_TO_SEND==true)
		//	CURRENT_DATA_IS_SENT=true;
		
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

	pMainApplication->collaboration_creator_res = ResIndex;
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
