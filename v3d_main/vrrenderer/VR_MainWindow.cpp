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
	


//    QDataStream in(VR_Communicator->socket);
//    in.setVersion(QDataStream::Qt_4_7);
//    QString line;

//    qDebug()<<"in MessageSocketSlot_Read TV:\n";
//    while(1)
//    {
//        if(VR_Communicator->nextblocksize==0)
//        {
//            if(VR_Communicator->socket->bytesAvailable()>=sizeof (quint64))
//            {
//                in>>VR_Communicator->nextblocksize;
//            }
//            else
//            {
//                return;
//            }
//        }

//        if(VR_Communicator->socket->bytesAvailable()>=VR_Communicator->nextblocksize)
//        {
//            in >>line;
//        }else
//        {
//            return ;
//        }

//        line=line.trimmed();
//        qDebug()<<"===TVProcess:"<<line;

        if (usersRex.indexIn(line) != -1) {
            QStringList users = usersRex.cap(1).split(",");
            //qDebug()<<"Current users are:";
            foreach (QString user, users) {
                //qDebug()<<user;
                if(user==userName) continue;// skip itself
                //traverse the user list. Create new item for Agents[] if there is a new agent.
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
            //QString msg = systemRex.cap(1);
            //qDebug()<<"System's Broadcast:"<<msg;
            QStringList sysMSGs = systemRex.cap(1).split(" ");
            if(sysMSGs.size()<2) return;
            //Update Agents[] on user login/logout
            QString user=sysMSGs.at(0);
            QString Action=sysMSGs.at(1);
            if((user!=userName)&&(Action=="joined"))
            {
                qDebug()<<"user: "<< user<<"joined";
                //the message is user ... joined
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
            //qDebug()<<"run hmd";
            //QString POSofHMD = hmdposRex.cap(1);
            QStringList hmdMSGs = hmdposRex.cap(1).split(" ");
            if(hmdMSGs.size()<17) return;

            QString user=hmdMSGs.at(0);
            if(user == userName) return;//the msg is the position of the current user,do nothing
            qDebug()<<"get user hmd pos info"<<"       "<<user;
            for(int i=0;i<VR_Communicator->Agents.size();i++)
            {
                if(user == VR_Communicator->Agents.at(i).name)// the msg is the position of user[i],update POS
                {
                    for(int j=0;j<16;j++)
                    {
                        VR_Communicator->Agents.at(i).position[j]=hmdMSGs.at(j+1).toFloat();
                        //qDebug("Agents.at(i).position[15]=%f",Agents.at(i).position[i]);
                        //qDebug()<<"Agent["<<i<<"] "<<" user: "<<Agents.at(i).name<<"HMD Position ="<<Agents.at(i).position[15];
                    }
                    break;
                }
            }
        }
        else if(colorRex.indexIn(line) != -1) {
            //qDebug()<<"run color";
            //QString colorFromServer = colorRex.cap(1);
            //qDebug()<<"the color receieved is :"<<colorFromServer;
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
        else if(creatorRex.indexIn(line) != -1) {
            qDebug()<<"get creator message";
            //QString colorFromServer = colorRex.cap(1);
            //qDebug()<<"the color receieved is :"<<colorFromServer;
            QStringList creatorMSGs = creatorRex.cap(1).split(" ");
            QString user=creatorMSGs.at(0);
            QString creator_Res = creatorMSGs.at(1);
            for(int i=0;i<VR_Communicator->Agents.size();i++)
            {
                qDebug()<<"creator name is "<<user;
                if(VR_Communicator->Agents.at(i).name!=user) continue;
                pMainApplication->collaboration_creator_name = user;
                pMainApplication->collaboration_creator_res = creator_Res.toInt();
                qDebug()<<"user:"<<user<<" receievedCreator"<<pMainApplication->collaboration_creator_name;
                qDebug()<<"user:"<<user<<" receievedCreator res"<<pMainApplication->collaboration_creator_res;
            }
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
                    qDebug()<<"xyz lsit::"<<xyz;
                    if(xyz.size()<6) continue;
					float resx = xyz.at(3).toFloat();
					float resy = xyz.at(4).toFloat();
					float resz = xyz.at(5).toFloat();
					pMainApplication->collaborationTargetdelcurveRes = XYZ(resx, resy, resz);
                    if(xyz.at(0).toFloat()<VRVolumeStartPoint.x ||xyz.at(1).toFloat()<VRVolumeStartPoint.y||xyz.at(2).toFloat()<VRVolumeStartPoint.z
                            ||xyz.at(0).toFloat()>VRVolumeEndPoint.x||xyz.at(1).toFloat()>VRVolumeEndPoint.y||xyz.at(2).toFloat()>VRVolumeEndPoint.z
                    )
                    {
                        qDebug()<<"====================out =====================";
                        VROutinfo.deletedcurvespos.push_back(XYZ(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat()));
                        continue;
                    }

                    XYZ node=ConvertreceiveCoords(xyz.at(0).toFloat(),xyz.at(1).toFloat(),xyz.at(2).toFloat());

                    pMainApplication->DeleteSegment(node.x,node.y,node.z);
                }
             }

                    qDebug()<<".................................";

//            QStringList delIDList;
//            for(int i=0;i<delMSGs.size();i++)
//            {
//                QStringList tempnodeList=delMSGs.at(i).split(" ",QString::SkipEmptyParts);

//                if(tempnodeList.size()<4) continue;
//                float dx = tempnodeList.at(0).toFloat();
//                float dy = tempnodeList.at(1).toFloat();
//                float dz = tempnodeList.at(2).toFloat();

//                if (pMainApplication)
////				{
////					pMainApplication->collaborationTargetdelcurveRes = XYZ(resx, resy, resz);

////					XYZ  converreceivexyz = ConvertreceiveCoords(dx, dy, dz);
////					XYZ TeraflyglobalPos = XYZ(dx, dy, dz);

////					QString delID = pMainApplication->FindNearestSegment(glm::vec3(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z));
////					delIDList.append(delID);

//                    pMainApplication->DeleteSegment(delIDList.at(i));
//                }
                


//            }
//			if (pMainApplication)
//			{
//				for (int i = 0; i < delIDList.size(); i++)
//				{
//					pMainApplication->DeleteSegment(delIDList.at(i));
//				}
//			}





            //            qDebug()<<"pMainApplication->collaborationTargetdelcurveRes = XYZ(resx,resy,resz);";
            //			qDebug()<<"user, "<<user<<" delete: "<<dx<<dy<<dz;

            //			qDebug()<<"user, "<<user<<" Converted Receive curve: "<<converreceivexyz.x<<" "<<converreceivexyz.y<<" "<<converreceivexyz.z;


            //			dx/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
            //			dy/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
            //			dz/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);

            //			if(TeraflyglobalPos.x<VRVolumeStartPoint.x ||
            //			TeraflyglobalPos.y<VRVolumeStartPoint.y||
            //			TeraflyglobalPos.z<VRVolumeStartPoint.z||
            //			TeraflyglobalPos.x>VRVolumeEndPoint.x||
            //			TeraflyglobalPos.y>VRVolumeEndPoint.y||
            //			TeraflyglobalPos.z>VRVolumeEndPoint.z
            //			)
            //			{
            //				qDebug()<<"push_back test delete point ";
            //				VROutinfo.deletedcurvespos.push_back(TeraflyglobalPos);
            //			}
            //			qDebug()<<"deletedcurvespos"<<dx<<" "<<dy<<" "<<dz;


            //            qDebug()<<"delete ID"<<delID<<"++++++++++++++++++++";
            //			bool delerror = pMainApplication->DeleteSegment(delID);

            //			if(delerror==true)
            //				qDebug()<<"Segment Deleted.";
            //			else
            //				qDebug()<<"Cannot Find the Segment ";
            // pMainApplication->MergeNeuronTrees();
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
                qDebug()<<"size < 4";
                return;
            }

            float mx = markerMSGs.at(0).toFloat();
            float my = markerMSGs.at(1).toFloat();
            float mz = markerMSGs.at(2).toFloat();
            int resx = markerMSGs.at(3).toFloat();
            int resy = markerMSGs.at(4).toFloat();
            int resz = markerMSGs.at(5).toFloat();
            qDebug()<<"user, "<<user<<" marker: "<<mx<<" "<<my<<" "<<mz;
            qDebug()<<"user, "<<user<<" Res: "<<resx<<" "<<resy<<" "<<resz;
			if (pMainApplication)
			{
				pMainApplication->CollaborationTargetMarkerRes = XYZ(resx, resy, resz);
				cout << "pos 1" << endl;
				XYZ  converreceivexyz = ConvertreceiveCoords(mx, my, mz);
				qDebug() << "user, " << user << " Converted Receive marker: " << converreceivexyz.x << " " << converreceivexyz.y << " " << converreceivexyz.z;
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


                        qDebug()<<"markerpos: "<<QString("%1 %2 %3").arg(mx).arg(my).arg(mz);
                        qDebug()<<"VR START: "<<QString("%1 %2 %3").arg(VRVolumeStartPoint.x).arg(VRVolumeStartPoint.y).arg(VRVolumeStartPoint.z);
                        qDebug()<<"VR END: "<<QString("%1 %2 %3").arg(VRVolumeEndPoint.x).arg(VRVolumeEndPoint.y).arg(VRVolumeEndPoint.z);
                if(mx<VRVolumeStartPoint.x || my<VRVolumeStartPoint.y||mz<VRVolumeStartPoint.z|| mx>VRVolumeEndPoint.x||my>VRVolumeEndPoint.y||mz>VRVolumeEndPoint.z)
                {
                    qDebug()<<"marker out of size";
                    VROutinfo.deletemarkerspos.push_back(QString("%1 %2 %3 %4").arg(mx).arg(my).arg(mz).arg(colortype));
                    return;
                }
				//pMainApplication->SetupMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype);
				bool IsmarkerValid = false;
				IsmarkerValid = pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z);
				cout << "IsmarkerValid is " << IsmarkerValid << endl;
				if (!IsmarkerValid)
				{
                        qDebug()<<"kljkllk";
					pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, colortype);
				}
			}

        }
        else if (delmarkerRex.indexIn(line) != -1) {
            QStringList delmarkerPOS = delmarkerRex.cap(2).split(" ",QString::SkipEmptyParts);
            if(delmarkerPOS.size()<3)
            {
                qDebug()<<"size < 4";
                return;
            }
            QString user = delmarkerRex.cap(1);
            float mx = delmarkerPOS.at(1).toFloat();
            float my = delmarkerPOS.at(2).toFloat();
            float mz = delmarkerPOS.at(3).toFloat();
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



            qDebug()<<"1126:current type ="<<colortype;
            pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype);
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
        //dragnodeRex
        else if (messageRex.indexIn(line) != -1) {
            //qDebug()<<"recive NO."<<numreceivedmessage<<" :"<<line;     //hl debug

            QString user=messageRex.cap(1);
            QStringList MSGs = messageRex.cap(2).split("_",QString::SkipEmptyParts);//list of nodes: seg header_node 1_node 2.....
            qDebug()<<MSGs[0];
            QString message;

            qDebug()<<"======================messageindex in TeraVr begin=================";
            for(int i=1;i<MSGs.size();i++)
            {
                //                qDebug()<<MSGs.at(i)<<endl;

                QString PointMSG=MSGs.at(i);
                QStringList poingmsg=PointMSG.trimmed().split(" ");
                XYZ LocCoords=ConvertreceiveCoords(poingmsg[2].toFloat(),poingmsg[3].toFloat(),poingmsg[4].toFloat());
                message+=QString(QString::number(i)+" "+poingmsg[1]+" "+QString::number(LocCoords.x)
                        +" "+QString::number(LocCoords.y)+" "+QString::number(LocCoords.z)+" "+poingmsg[5]+" ");
                if(i==1) message+=QString::number(-1);
                else message+=QString::number(i-1);

                if(i!=MSGs.size()-1) message+=" ";

            }
            qDebug()<<"======================messageindex in TeraVr end=================";
            qDebug()<<"user, "<<user<<" said: "<<message;
            if(pMainApplication)
            {
                if(user==userName)
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    qDebug()<<"liqiqiqiqiqiqiqi NT "<<endl;
                }

                int colortype;
                for(int i=0;i<VR_Communicator->Agents.size();i++)
                {
                    if(user == VR_Communicator->Agents.at(i).name)
                    {
                        colortype=VR_Communicator->Agents.at(i).colorType;
                        break;
                    }
                }
                pMainApplication->UpdateNTList(message,colortype);
            }
        }

//        VR_Communicator->nextblocksize=0;
//    }
//    }
}

//void VR_MainWindow::onConnected() {

//    float m_globalScale=pMainApplication->get_mglobalScal();

//    VR_Communicator->onReadySend(QString("/login:" +userName ));
////    VR_Communicator->socket->write(QString("/global:" +QString("%1").arg(m_globalScale) +"\n").toUtf8());
//}

//void VR_MainWindow::onDisconnected() {
//    qDebug("Now disconnect with the server.");
//	//Agents.clear();
//	emit VRSocketDisconnect();
//	if(pMainApplication)
//		pMainApplication->isOnline = false;
//	this->close();
	
//}



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
        qDebug()<<"pMainApplication->Shutdown();123";
//        disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
//        connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
        qDebug()<<"pMainApplication->Shutdown();1234";
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
		CreatorPos->x = pMainApplication->CollaborationCreatorPos.x;
		CreatorPos->y = pMainApplication->CollaborationCreatorPos.y;
		CreatorPos->z = pMainApplication->CollaborationCreatorPos.z;
		CreatorRes = pMainApplication->collaboration_creator_res;
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
            QString waitsend=pMainApplication->NT2QString();
            if(waitsend.size()!=0)
                CollaborationSendPool.emplace_back(pMainApplication->NT2QString());
			pMainApplication->ClearCurrentNT();
            emit sendPoolHead();
			CURRENT_DATA_IS_SENT=true;
			qDebug()<<"CURRENT_DATA_IS_SENT=true;";
		}
		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
		{

			qDebug()<<"delname = "<<pMainApplication->delName;
			qDebug()<<"delcurvePOS = "<<pMainApplication->delcurvePOS;
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
			qDebug()<<"marker position = "<<pMainApplication->markerPOS;
			QString ConvertedmarkerPOS = ConvertsendCoords(pMainApplication->markerPOS);
			qDebug()<<"Converted marker position = "<<ConvertedmarkerPOS;
			QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
            VR_Communicator->onReadySend(QString("/marker:" + ConvertedmarkerPOS +" "+QSCurrentRes ));
			CURRENT_DATA_IS_SENT=true;
		}
		//else if(pMainApplication->m_modeGrip_R==m_delmarkMode)
		//{
		//	qDebug()<<"marker to be delete position = "<<pMainApplication->delmarkerPOS;
		//	QString ConverteddelmarkerPOS = ConvertsendCoords(pMainApplication->delmarkerPOS);
		//	qDebug()<<"Converted delete marker position = "<<ConverteddelmarkerPOS;
		//	socket->write(QString("/del_marker:" + ConverteddelmarkerPOS + "\n").toUtf8());
		//	CURRENT_DATA_IS_SENT=true;
		//}
		else if(pMainApplication->m_modeGrip_R==m_dragMode)
		{
			qDebug()<<"drag node new position = "<<pMainApplication->dragnodePOS;
			QString ConverteddragnodePOS = ConvertsendCoords(pMainApplication->dragnodePOS);
			qDebug()<<"Converted delete marker position = "<<ConverteddragnodePOS;
            VR_Communicator->onReadySend(QString("/drag_node:" + ConverteddragnodePOS ));
			CURRENT_DATA_IS_SENT=true;
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
	qDebug()<<"GetResindexandStartPointfromVRInfo........";
    qDebug()<<"VRinfo"<<VRinfo;
	QRegExp rx("Res\\((\\d+)\\s.\\s(\\d+)\\s.\\s(\\d+)\\),Volume\\sX.\\[(\\d+),(\\d+)\\],\\sY.\\[(\\d+),(\\d+)\\],\\sZ.\\[(\\d+),(\\d+)\\]");   
	if (rx.indexIn(VRinfo) != -1 && (ResIndex != -1)) {
		qDebug()<<"get  VRResindex and VRVolume Start point ";
		VRVolumeStartPoint = XYZ(rx.cap(4).toInt(),rx.cap(6).toInt(),rx.cap(8).toInt());
		VRVolumeEndPoint = XYZ(rx.cap(5).toInt(),rx.cap(7).toInt(),rx.cap(9).toInt());
		VRVolumeCurrentRes = XYZ(rx.cap(1).toInt(),rx.cap(2).toInt(),rx.cap(3).toInt());
		VRvolumeMaxRes = CollaborationMaxResolution;
		qDebug()<<"get Resindex = "<<ResIndex;
		qDebug()<<"Start X = "<<VRVolumeStartPoint.x<<"Start Y = "<<VRVolumeStartPoint.y<<"Start Z = "<<VRVolumeStartPoint.z;
		qDebug()<<"End X = "<<VRVolumeEndPoint.x<<"End Y = "<<VRVolumeEndPoint.y<<"End Z = "<<VRVolumeEndPoint.z;
		qDebug()<<"current Res X = "<<VRVolumeCurrentRes.x<<"current Res Y = "<<VRVolumeCurrentRes.y<<"current Res Z = "<<VRVolumeCurrentRes.z;
		qDebug()<<"Collaboration Max Res X = "<<CollaborationMaxResolution.x<<"Collaboration Max Y = "<<CollaborationMaxResolution.y<<"Collaboration Max Z = "<<CollaborationMaxResolution.z;
	}
	else
	{
		VRVolumeStartPoint = XYZ(1,1,1);
		VRVolumeEndPoint = CollaborationMaxResolution;
		VRVolumeCurrentRes = CollaborationMaxResolution;
		VRvolumeMaxRes = CollaborationMaxResolution;
		qDebug()<<"get Resindex = "<<ResIndex;
		qDebug()<<"Start X = "<<VRVolumeStartPoint.x<<"Start Y = "<<VRVolumeStartPoint.y<<"Start Z = "<<VRVolumeStartPoint.z;
		qDebug()<<"End X = "<<VRVolumeEndPoint.x<<"End Y = "<<VRVolumeEndPoint.y<<"End Z = "<<VRVolumeEndPoint.z;
		qDebug()<<"current Res X = "<<VRVolumeCurrentRes.x<<"current Res Y = "<<VRVolumeCurrentRes.y<<"current Res Z = "<<VRVolumeCurrentRes.z;
		qDebug()<<"Collaboration Max Res X = "<<CollaborationMaxResolution.x<<"Collaboration Max Y = "<<CollaborationMaxResolution.y<<"Collaboration Max Z = "<<CollaborationMaxResolution.z;

	}
	//pass Resindex and VRvolumeStartPoint to PMAIN  to  offer parameter to NT2QString
	pMainApplication->CmainResIndex = ResIndex;
    pMainApplication->CmainVRVolumeStartPoint = VRVolumeStartPoint;
	pMainApplication->collaboration_creator_res = ResIndex;
	cout<<"pMainApplication->collaboration_creator_res = "<<pMainApplication->collaboration_creator_res<<endl;
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
	cout << "pos 2" << endl;
	cout << "current x = " << VRVolumeCurrentRes.x << "current y = " << VRVolumeCurrentRes.y << "current z = " << VRVolumeCurrentRes.z << endl;
	cout << "max x = " << VRvolumeMaxRes.x << "max y = " << VRvolumeMaxRes.y << "max z = " << VRvolumeMaxRes.z << endl;
	float dividex = VRvolumeMaxRes.x/VRVolumeCurrentRes.x;
	cout << "dividex =" << dividex;

	float dividey = VRvolumeMaxRes.y/VRVolumeCurrentRes.y;
	float dividez = VRvolumeMaxRes.z/VRVolumeCurrentRes.z;
	cout<<"dividex = "<<dividex<<"dividey = "<<dividey<<"dividez = "<<dividez<<endl;
	x/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
	y/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
	z/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    cout<<" x = "<<x<<"y = "<<y<<"z = "<<z<<endl;
	x-=(VRVolumeStartPoint.x-1);
	y-=(VRVolumeStartPoint.y-1);
	z-=(VRVolumeStartPoint.z-1);
	return XYZ(x,y,z);
}
