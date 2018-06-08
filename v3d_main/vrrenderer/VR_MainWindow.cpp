#include "VR_MainWindow.h"

#include "v3dr_gl_vr.h"
#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>

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

bool VR_MainWindow::SendLoginRequest() {

    QSettings settings("HHMI", "Vaa3D");
    QString serverNameDefault = "";
	if(!settings.value("vr_serverName").toString().isEmpty())
		serverNameDefault = settings.value("vr_serverName").toString();
	bool ok1;
	QString serverName = QInputDialog::getText(0, "Server Address",
		"Please enter the server address:", QLineEdit::Normal,
		serverNameDefault, &ok1);

	if(ok1 && !serverName.isEmpty())
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
			return SendLoginRequest();
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
				return SendLoginRequest();
			}else
				settings.setValue("vr_userName", userName);
		}

		Agent agent00={
			//with local information
			userName,
			true,//means this struct point to itself,no need to render
			21,
			0
		};
		Agents.push_back(agent00);

	}
    else
    {
        qDebug()<<"WRONG!EMPTY! ";
        return SendLoginRequest();
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
					0 //POS
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
        else if (deletecurveRex.indexIn(line) != -1) {
			QStringList delMSGs = deletecurveRex.cap(1).split(" ");
			if(delMSGs.size()<2) 
			{
					qDebug()<<"size < 2";
					return;
			}
            QString user = delMSGs.at(0);
            QString delID = delMSGs.at(1);
			qDebug()<<"user, "<<user<<" delete: "<<delID;
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
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
			qDebug()<<"user, "<<user<<" marker: "<<mx<<" "<<my<<" "<<mz;
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
			pMainApplication->SetupMarkerandSurface(mx,my,mz,colortype);
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
			pMainApplication->RemoveMarkerandSurface(mx,my,mz,colortype);
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
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
			pMainApplication->UpdateDragNodeinNTList(ntnum,swcnum,mx,my,mz);
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



void VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain, bool isLinkSuccess) {

	pMainApplication = new CMainApplication( 0, 0 );

	pMainApplication->mainwindow =pmain; 

	pMainApplication->isOnline = isLinkSuccess;
    //pMainApplication->loadedNT.listNeuron.clear();
    //pMainApplication->loadedNT.hashNeuron.clear();
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
		return;
	}
	RunVRMainloop();
	QTimer::singleShot(3000, this, SLOT(SendHMDPosition()));
}
void VR_MainWindow::SendHMDPosition()
{
	if(!pMainApplication) return;
	//get hmd position
	QString PositionStr=pMainApplication->getHMDPOSstr();

	//send hmd position
	socket->write(QString("/hmdpos:" + PositionStr + "\n").toUtf8());

	QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
}
void VR_MainWindow::RunVRMainloop()
{
	//update agents position if necessary
	if(Agents.size()>0)
		pMainApplication->SetupAgentModels(Agents);

	//handle one rendering loop, and handle user interaction
	bool bQuit=pMainApplication->HandleOneIteration();

	if(bQuit==true)
	{
		qDebug()<<"Now quit VR";
		socket->disconnectFromHost();
		Agents.clear();
		delete pMainApplication;
		pMainApplication=0;
		return;
	}

	//send local data to server
	if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
	//READY_TO_SEND is set to true by the "trigger button up" event;
	//client sends data to server (using onReadySend());
	//server sends the same data back to client;
	//READY_TO_SEND is set to false in onReadyRead();
	//CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
	{
		if(pMainApplication->m_modeGrip_R==m_drawMode)
			onReadySend(pMainApplication->NT2QString());
		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
		{
			qDebug()<<"delname = "<<pMainApplication->delName;
			if(pMainApplication->delName!="")
				socket->write(QString("/del_curve:" + pMainApplication->delName + "\n").toUtf8());
			else
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}
		}
		else if(pMainApplication->m_modeGrip_R==m_markMode)
		{
			qDebug()<<"marker position = "<<pMainApplication->markerPOS;
			socket->write(QString("/marker:" + pMainApplication->markerPOS + "\n").toUtf8());
		}
		else if(pMainApplication->m_modeGrip_R==m_delmarkMode)
		{
			qDebug()<<"marker to be delete position = "<<pMainApplication->delmarkerPOS;
			socket->write(QString("/del_marker:" + pMainApplication->delmarkerPOS + "\n").toUtf8());
		}
		else if(pMainApplication->m_modeGrip_R==m_dragMode)
		{
			qDebug()<<"drag node new position = "<<pMainApplication->dragnodePOS;
			socket->write(QString("/drag_node:" + pMainApplication->dragnodePOS + "\n").toUtf8());
		}
		if(pMainApplication->READY_TO_SEND==true)
			CURRENT_DATA_IS_SENT=true;
	}


	QTimer::singleShot(20, this, SLOT(RunVRMainloop()));
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


