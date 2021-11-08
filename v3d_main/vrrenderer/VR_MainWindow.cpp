#include "VR_MainWindow.h"
#include "v3dr_gl_vr.h"
#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>
#include <math.h>
#include <QInputDialog>
std::vector<Agent> Agents;
VR_MainWindow::VR_MainWindow() :
	QWidget()
{
	if (Agents.size()>0)
		Agents.clear();
	userName="";
	QRegExp regex("^[a-zA-Z]\\w+");
	socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	CURRENT_DATA_IS_SENT=false;
}

VR_MainWindow::~VR_MainWindow() {

}

bool VR_MainWindow::SendLoginRequest(bool resume) {

    QSettings settings("HHMI", "Vaa3D");
    QString serverNameDefault = "";
	if(!settings.value("vr_serverName").toString().isEmpty())
		serverNameDefault = settings.value("vr_serverName").toString();
	QString serverName;
	bool ok1;
	if(!resume)
	{serverName = QInputDialog::getText(0, "Server Address",
		"Please enter the server address:", QLineEdit::Normal,
		serverNameDefault, &ok1);
	if(!ok1 || serverName.isEmpty())
		{
			qDebug()<<"WRONG!EMPTY! ";
			//return SendLoginRequest();
			return 0;
	}
	else
	{
		settings.setValue("vr_serverName", serverName);
		QString PortDefault = "";
		if(!settings.value("vr_PORT").toString().isEmpty())
			PortDefault = settings.value("vr_PORT").toString();
		bool ok2;
		vr_Port = QInputDialog::getText(0, "Port",
			"Please enter server port:", QLineEdit::Normal,
			PortDefault, &ok2);

		if(!ok2 || vr_Port.isEmpty())
		{
			qDebug()<<"WRONG!EMPTY! ";
			//return SendLoginRequest();
			return 0;
		}
		else
		{
			settings.setValue("vr_PORT", vr_Port);
			QString userNameDefault = "";
			if(!settings.value("vr_userName").toString().isEmpty())
				userNameDefault = settings.value("vr_userName").toString();
			bool ok3;
			userName = QInputDialog::getText(0, "Lgoin Name",
				"Please enter your login name:", QLineEdit::Normal,
				userNameDefault, &ok3);

			if(!ok3 || userName.isEmpty())
			{
				qDebug()<<"WRONG!EMPTY! ";
				//return SendLoginRequest();
				return 0;
			}else
				settings.setValue("vr_userName", userName);
		}

		Agent agent00={
			//with local information
			userName,
			true,//means this struct point to itself,no need to render
			21,
			0,
		};
		Agents.push_back(agent00);

	}

	}
	else
	{
		serverName = serverNameDefault;
		QString PortDefault = "";
		if(!settings.value("vr_PORT").toString().isEmpty())
			PortDefault = settings.value("vr_PORT").toString();
		vr_Port = PortDefault;
		QString userNameDefault = "";
			if(!settings.value("vr_userName").toString().isEmpty())
				userNameDefault = settings.value("vr_userName").toString();
			userName = userNameDefault;
			settings.setValue("vr_userName", userName);
			Agent agent00={
			//with local information
			userName,
			true,//means this struct point to itself,no need to render
			21,
			0
		};
		Agents.push_back(agent00);

	}
    socket->connectToHost(serverName, vr_Port.toUInt());
	if(!socket->waitForConnected(15000))
	{
		if(socket->state()==QAbstractSocket::UnconnectedState)
		{
			qDebug()<<"Cannot connect with Server. Unknown error.";
			return 0;
		}	
	}
	qDebug()<<"User:  "<<userName<<".  Connected with server: "<<serverName<<" :"<<vr_Port;
	return 1;
}

void VR_MainWindow::onReadySend(QString &send_MSG) {

    if (!send_MSG.isEmpty()) {
		if((send_MSG!="exit")&&(send_MSG!="quit"))
		{
			socket->write(QString("/say:" + send_MSG + "\n").toUtf8());
		}
		else
		{
			socket->write(QString("/say: GoodBye~\n").toUtf8());
			socket->disconnectFromHost();
			return;
		}
	}
	else
	{
		qDebug()<<"The message is empty!";
		//on_pbSend_clicked();
	}
}

void VR_MainWindow::onReadyRead() {
    QRegExp usersRex("^/users:(.*)$");
    QRegExp systemRex("^/system:(.*)$");
	QRegExp hmdposRex("^/hmdpos:(.*)$");
	QRegExp colorRex("^/color:(.*)$");
	QRegExp deletecurveRex("^/del_curve:(.*)$");
	QRegExp markerRex("^/marker:(.*)$");
	QRegExp delmarkerRex("^/del_marker:(.*)$");
	QRegExp dragnodeRex("^/drag_node:(.*)$");
	QRegExp creatorRex("^/creator:(.*)$");
    QRegExp messageRex("^(.*):(.*)$");
	

    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();

        if (usersRex.indexIn(line) != -1) {
            QStringList users = usersRex.cap(1).split(",");
			//qDebug()<<"Current users are:";
            foreach (QString user, users) {
				//qDebug()<<user;
				if(user==userName) continue;// skip itself
				//traverse the user list. Create new item for Agents[] if there is a new agent.
				bool findSameAgent=false;
				for(int i=0;i<Agents.size();i++)
				{
					if(user==Agents.at(i).name)
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
					Agents.push_back(agent00);
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
				Agents.push_back(agent00);
			}
			else if((user!=userName)&&(Action=="left"))
			{
				qDebug()<<"user: "<< user<<"left";
				//the message is user ... left
				for(int i=0;i<Agents.size();i++)
				{
					if(user == Agents.at(i).name)
					{
						//qDebug()<<"before erase "<<Agents.size();
						Agents.erase(Agents.begin()+i);
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
			for(int i=0;i<Agents.size();i++)
			{		
				if(user == Agents.at(i).name)// the msg is the position of user[i],update POS
				{
					for(int j=0;j<16;j++)
					{
						Agents.at(i).position[j]=hmdMSGs.at(j+1).toFloat();
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
			for(int i=0;i<Agents.size();i++)
			{
				if(Agents.at(i).name!=user) continue;
					//update agent color
				Agents.at(i).colorType=clrtype.toInt();
				qDebug()<<"user:"<<user<<" receievedColorTYPE="<<Agents.at(i).colorType;
				if(user == userName)
					pMainApplication->SetupCurrentUserInformation(userName.toStdString(), Agents.at(i).colorType);
			}
		}
		else if(creatorRex.indexIn(line) != -1) {
			qDebug()<<"get creator message";
			//QString colorFromServer = colorRex.cap(1);
			//qDebug()<<"the color receieved is :"<<colorFromServer;
			QStringList creatorMSGs = creatorRex.cap(1).split(" ");
			QString user=creatorMSGs.at(0);
			QString creator_Res = creatorMSGs.at(1);
			for(int i=0;i<Agents.size();i++)
			{
				qDebug()<<"creator name is "<<user;
				if(Agents.at(i).name!=user) continue;
				pMainApplication->collaboration_creator_name = user;
				pMainApplication->collaboration_creator_res = creator_Res.toInt();
				qDebug()<<"user:"<<user<<" receievedCreator"<<pMainApplication->collaboration_creator_name;
				qDebug()<<"user:"<<user<<" receievedCreator res"<<pMainApplication->collaboration_creator_res;
			}
		}
        else if (deletecurveRex.indexIn(line) != -1) {
			QStringList delMSGs = deletecurveRex.cap(1).split(" ");
			if(delMSGs.size()<2) 
			{
					qDebug()<<"size < 2";
					return;
			}
            QString user = delMSGs.at(0);
            float dx = delMSGs.at(1).toFloat();
			float dy = delMSGs.at(2).toFloat();
			float dz = delMSGs.at(3).toFloat();
            float resx = delMSGs.at(4).toFloat();
			float resy = delMSGs.at(5).toFloat();
			float resz = delMSGs.at(6).toFloat();

			pMainApplication->collaborationTargetdelcurveRes = XYZ(resx,resy,resz);
			qDebug()<<"user, "<<user<<" delete: "<<dx<<dy<<dz;
			XYZ  converreceivexyz = ConvertreceiveCoords(dx,dy,dz);
			qDebug()<<"user, "<<user<<" Converted Receive curve: "<<converreceivexyz.x<<" "<<converreceivexyz.y<<" "<<converreceivexyz.z;
			XYZ TeraflyglobalPos =XYZ(dx ,dy,dz);
			dx/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
			dy/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
			dz/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
			
			if(TeraflyglobalPos.x<VRVolumeStartPoint.x || 
			TeraflyglobalPos.y<VRVolumeStartPoint.y||
			TeraflyglobalPos.z<VRVolumeStartPoint.z||
			TeraflyglobalPos.x>VRVolumeEndPoint.x||
			TeraflyglobalPos.y>VRVolumeEndPoint.y||
			TeraflyglobalPos.z>VRVolumeEndPoint.z
			)
			{
				qDebug()<<"push_back test delete point ";
				VROutinfo.deletedcurvespos.push_back(TeraflyglobalPos);
			}
			qDebug()<<"deletedcurvespos"<<dx<<" "<<dy<<" "<<dz;
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
			QString delID = pMainApplication->FindNearestSegment(glm::vec3(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z));
			qDebug()<<"delete ID"<<delID;
			bool delerror = pMainApplication->DeleteSegment(delID);
			if(delerror==true)
				qDebug()<<"Segment Deleted.";
			else
				qDebug()<<"Cannot Find the Segment ";
			// pMainApplication->MergeNeuronTrees();
        }
        else if (markerRex.indexIn(line) != -1) {
			QStringList markerMSGs = markerRex.cap(1).split(" ");
			if(markerMSGs.size()<4) 
			{
					qDebug()<<"size < 4";
					return;
			}
            QString user = markerMSGs.at(0);
            float mx = markerMSGs.at(1).toFloat();
			float my = markerMSGs.at(2).toFloat();
			float mz = markerMSGs.at(3).toFloat();
			int resx = markerMSGs.at(4).toFloat();
			int resy = markerMSGs.at(5).toFloat();
			int resz = markerMSGs.at(6).toFloat();	
			qDebug()<<"user, "<<user<<" marker: "<<mx<<" "<<my<<" "<<mz;
			qDebug()<<"user, "<<user<<" Res: "<<resx<<" "<<resy<<" "<<resz;
			pMainApplication->CollaborationTargetMarkerRes = XYZ(resx,resy,resz);
			XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
			qDebug()<<"user, "<<user<<" Converted Receive marker: "<<converreceivexyz.x<<" "<<converreceivexyz.y<<" "<<converreceivexyz.z;
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				qDebug()<<"get message CURRENT_DATA_IS_SENT=false;";
				pMainApplication->ClearCurrentNT();
			}
			int colortype=3;
			for(int i=0;i<Agents.size();i++)
			{
				if(user == Agents.at(i).name)
				{
					colortype=Agents.at(i).colorType;
					break;
				}
			}
			//pMainApplication->SetupMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype);
			bool IsmarkerValid = false;
			IsmarkerValid = pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z);
			cout<<"IsmarkerValid is "<<IsmarkerValid<<endl;
			if(!IsmarkerValid)
			{
				pMainApplication->SetupMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype);
			}

        }
        else if (delmarkerRex.indexIn(line) != -1) {
			QStringList delmarkerPOS = delmarkerRex.cap(1).split(" ");
			if(delmarkerPOS.size()<4) 
			{
					qDebug()<<"size < 4";
					return;
			}
            QString user = delmarkerPOS.at(0);
            float mx = delmarkerPOS.at(1).toFloat();
			float my = delmarkerPOS.at(2).toFloat();
			float mz = delmarkerPOS.at(3).toFloat();
			qDebug()<<"user, "<<user<<"del marker: "<<mx<<" "<<my<<" "<<mz;
			XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
			int colortype=3;
			for(int i=0;i<Agents.size();i++)
			{
				if(user == Agents.at(i).name)
				{
					colortype=Agents.at(i).colorType;
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
            QString user = messageRex.cap(1);
            QString message = messageRex.cap(2);
			//qDebug()<<"user, "<<user<<" said: "<<message;
			if(pMainApplication)
			{
				if(user==userName)
				{
					pMainApplication->READY_TO_SEND=false;
					CURRENT_DATA_IS_SENT=false;
					pMainApplication->ClearCurrentNT();
				}

				int colortype;
				for(int i=0;i<Agents.size();i++)
				{
					if(user == Agents.at(i).name)
					{
						colortype=Agents.at(i).colorType;
						break;
					}
				}
				pMainApplication->UpdateNTList(message,colortype);
			}
		}
    }
}

void VR_MainWindow::onConnected() {

    socket->write(QString("/login:" +userName + "\n").toUtf8());

}

void VR_MainWindow::onDisconnected() {
    qDebug("Now disconnect with the server."); 
	//Agents.clear();
	emit VRSocketDisconnect();
	if(pMainApplication)
		pMainApplication->isOnline = false;
	this->close();
	
}



int VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain, bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,XYZ* zoomPOS,XYZ *CreatorPos,XYZ MaxResolution) {

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
		return 0;
	}
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
		socket->disconnectFromHost();
		Agents.clear();
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
	socket->write(QString("/hmdpos:" + PositionStr + "\n").toUtf8());
	//QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
	//cout<<"socket resindex"<<ResIndex<<endl;
	//qDebug()<<"QString resindex"<< QString("%1").arg(ResIndex);
	socket->write(QString("/ResIndex:" + QString("%1").arg(ResIndex) + "\n").toUtf8());
}
void VR_MainWindow::RunVRMainloop(XYZ* zoomPOS)
{
	qDebug()<<"get into RunMainloop";
	bool bQuit = false;
	int sendHMDPOScout = 0;
	while(!bQuit)
	{
	//update agents position if necessary
	if(Agents.size()>0)
		pMainApplication->SetupAgentModels(Agents);

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
            auto tmp=pMainApplication->NT2QString();
            onReadySend(tmp);
			CURRENT_DATA_IS_SENT=true;
			qDebug()<<"CURRENT_DATA_IS_SENT=true;";
		}
		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
		{

			qDebug()<<"delname = "<<pMainApplication->delName;
			qDebug()<<"delcurvePOS = "<<pMainApplication->delcurvePOS;
			if(pMainApplication->delName!="")
				//socket->write(QString("/del_curve:" + pMainApplication->delName + "\n").toUtf8());
			//else
			{
				QString ConverteddelcurvePOS = ConvertsendCoords(pMainApplication->delcurvePOS);
				qDebug()<<"Converted marker position = "<<ConverteddelcurvePOS;
				QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
				socket->write(QString("/del_curve:" +  ConverteddelcurvePOS+" "+QSCurrentRes + "\n").toUtf8());
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
			socket->write(QString("/marker:" + ConvertedmarkerPOS +" "+QSCurrentRes + "\n").toUtf8());
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
			socket->write(QString("/drag_node:" + ConverteddragnodePOS + "\n").toUtf8());
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
		socket->write(QString("/ask:message \n").toUtf8());
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
    qDebug()<<"shut down VR";
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
	qDebug()<<VRinfo;
	QRegExp rx("Res\\((\\d+)\\s.\\s(\\d+)\\s.\\s(\\d+)\\),Volume\\sX.\\[(\\d+),(\\d+)\\],\\sY.\\[(\\d+),(\\d+)\\],\\sZ.\\[(\\d+),(\\d+)\\]");   
	if (rx.indexIn(VRinfo) != -1) {
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
XYZ VR_MainWindow:: ConvertreceiveCoords(float x,float y,float z)
{
	//QString str1 = coords.section(' ',0, 0);  // str == "bin/myapp"
	//QString str2 = coords.section(' ',1, 1);  // str == "bin/myapp"
	//QString str3 = coords.section(' ',2, 2);  // str == "bin/myapp"
	float dividex = VRvolumeMaxRes.x/VRVolumeCurrentRes.x;
	float dividey = VRvolumeMaxRes.y/VRVolumeCurrentRes.y;
	float dividez = VRvolumeMaxRes.z/VRVolumeCurrentRes.z;
	cout<<"dividex = "<<dividex<<"dividey = "<<dividey<<"dividez = "<<dividez<<endl;
	x/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
	y/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
	z/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
	cout<<" x = "<<"y = "<<y<<"z = "<<z<<endl;
	x-=(VRVolumeStartPoint.x-1);
	y-=(VRVolumeStartPoint.y-1);
	z-=(VRVolumeStartPoint.z-1);
	return XYZ(x,y,z);
}
