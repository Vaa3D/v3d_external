#include "V3dR_Communicator.h"

#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>
#include <QInputDialog>

struct Agent {
	QString name;
	bool isItSelf;
	int colorType;
	float position[16];

};
static std::vector<Agent> Agents;
V3dR_Communicator::V3dR_Communicator(bool *client_flag, QList<NeuronTree>* ntlist) :
	QWidget()
{
	clienton = client_flag;
	NTList_3Dview = ntlist;
	NTNumReceieved=0;
	userName="";
	QRegExp regex("^[a-zA-Z]\\w+");
	socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	CURRENT_DATA_IS_SENT=false;
}

V3dR_Communicator::~V3dR_Communicator() {

}

bool V3dR_Communicator::SendLoginRequest() {

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

void V3dR_Communicator::onReadySend(QString &send_MSG) {

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

void V3dR_Communicator::onReadyRead() {
    QRegExp usersRex("^/users:(.*)$");
    QRegExp systemRex("^/system:(.*)$");
	//QRegExp hmdposRex("^/hmdpos:(.*)$");
	QRegExp colorRex("^/color:(.*)$");
	//QRegExp deleteRex("^/del:(.*)$");
	QRegExp markerRex("^/marker:(.*)$");
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
		//else if(hmdposRex.indexIn(line) != -1) {
		//	//qDebug()<<"run hmd";
		//	//QString POSofHMD = hmdposRex.cap(1);
		//	QStringList hmdMSGs = hmdposRex.cap(1).split(" ");
		//	if(hmdMSGs.size()<17) return;

		//	QString user=hmdMSGs.at(0);
		//	if(user == userName) return;//the msg is the position of the current user,do nothing 
		//	for(int i=0;i<Agents.size();i++)
		//	{		
		//		if(user == Agents.at(i).name)// the msg is the position of user[i],update POS
		//		{
		//			for(int j=0;j<16;j++)
		//			{
		//				Agents.at(i).position[j]=hmdMSGs.at(j+1).toFloat();
		//				//qDebug("Agents.at(i).position[15]=%f",Agents.at(i).position[i]);
		//				//qDebug()<<"Agent["<<i<<"] "<<" user: "<<Agents.at(i).name<<"HMD Position ="<<Agents.at(i).position[15];				
		//			}
		//			break;
		//		}
		//	}
		//}
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
			}
		}
   //     else if (deleteRex.indexIn(line) != -1) {
			//QStringList delMSGs = deleteRex.cap(1).split(" ");
			//if(delMSGs.size()<2) 
			//{
			//		qDebug()<<"size < 2";
			//		return;
			//}
   //         QString user = delMSGs.at(0);
   //         QString delID = delMSGs.at(1);
			//qDebug()<<"user, "<<user<<" delete: "<<delID;
			//if(user==userName)
			//{
			//	pMainApplication->READY_TO_SEND=false;
			//	CURRENT_DATA_IS_SENT=false;
			//	pMainApplication->ClearSketchNT();
			//}
			//bool delerror = pMainApplication->DeleteSegment(delID);
			//if(delerror==true)
			//	qDebug()<<"Segment Deleted.";
			//else
			//	qDebug()<<"Cannot Find the Segment ";
			//pMainApplication->MergeNTList2remoteNT();
   //     }
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
			int colortype=3;
			for(int i=0;i<Agents.size();i++)
			{
				if(user == Agents.at(i).name)
				{
					colortype=Agents.at(i).colorType;
					break;
				}
			}
        }
        else if (messageRex.indexIn(line) != -1) {
            QString user = messageRex.cap(1);
            QString message = messageRex.cap(2);
			//qDebug()<<"user, "<<user<<" said: "<<message;
			int colortype;
			for(int i=0;i<Agents.size();i++)
			{
				if(user == Agents.at(i).name)
				{
					colortype=Agents.at(i).colorType;
					break;
				}
			}
			qDebug()<<"receieved message :"<<message<<"  from user: "<<user<<"  type :"<<colortype;
			//trans message to neurontree with colortype
			//pMainApplication->UpdateNTList(message,colortype);
			qDebug()<<"loadedNTList.size()"<<NTList_3Dview->size();
			Update3DViewNTList(message,colortype);
			qDebug()<<"loadedNTList.size()"<<NTList_3Dview->size();
		}
    }
}

void V3dR_Communicator::onConnected() {

    socket->write(QString("/login:" +userName + "\n").toUtf8());

}

void V3dR_Communicator::onDisconnected() {
    qDebug("Now disconnect with the server."); 
	*clienton = false;
	//Agents.clear();
	this->close();

}


void V3dR_Communicator::Update3DViewNTList(QString &msg, int type)//may need to be changed to AddtoNTList( , )
{	
    QStringList qsl = QString(msg).trimmed().split(" ",Qt::SkipEmptyParts);
	int str_size = qsl.size()-(qsl.size()%7);//to make sure that the string list size always be 7*N;
	//qDebug()<<"qsl.size()"<<qsl.size()<<"str_size"<<str_size;
	NeuronSWC S_temp;
	NeuronTree tempNT;
	tempNT.listNeuron.clear();
	tempNT.hashNeuron.clear();
	//each segment has a unique ID storing as its name
	tempNT.name  = "sketch_"+ QString("%1").arg(NTNumReceieved++);
	for(int i=0;i<str_size;i++)
	{
		qsl[i].truncate(99);
		//qDebug()<<qsl[i];
		int iy = i%7;
		if (iy==0)
		{
			S_temp.n = qsl[i].toInt();
		}
		else if (iy==1)
		{
			S_temp.type = type;
		}
		else if (iy==2)
		{
			S_temp.x = qsl[i].toFloat();

		}
		else if (iy==3)
		{
			S_temp.y = qsl[i].toFloat();

		}
		else if (iy==4)
		{
			S_temp.z = qsl[i].toFloat();

		}
		else if (iy==5)
		{
			S_temp.r = qsl[i].toFloat();

		}
		else if (iy==6)
		{
			S_temp.pn = qsl[i].toInt();

			tempNT.listNeuron.append(S_temp);
			tempNT.hashNeuron.insert(S_temp.n, tempNT.listNeuron.size()-1);
		}
	}//*/
	//RGBA8 tmp= {(unsigned int)type};
	//tempNT.color = tmp;
	tempNT.color.i=type;
	NTList_3Dview->push_back(tempNT);
	qDebug()<<"receieved nt name is "<<tempNT.name;
	//updateremoteNT
}


//void V3dR_Communicator::SendHMDPosition()
//{
//	if(!pMainApplication) return;
//	//get hmd position
//	QString PositionStr=pMainApplication->getHMDPOSstr();
//
//	//send hmd position
//	socket->write(QString("/hmdpos:" + PositionStr + "\n").toUtf8());
//
//	QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
//}
//void V3dR_Communicator::RunVRMainloop()
//{
//
//	//handle one rendering loop, and handle user interaction
//	bool bQuit;//=HandleOneIteration();
//
//	if(bQuit==true)
//	{
//		qDebug()<<"Now quit VR";
//		socket->disconnectFromHost();
//		Agents.clear();
//		return;
//	}
//
//	//send local data to server
//	if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
//	{
//		if(pMainApplication->m_modeGrip_R==m_drawMode)
//			onReadySend(pMainApplication->NT2QString());
//		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
//		{
//			qDebug()<<"delname = "<<pMainApplication->delName;
//			if(pMainApplication->delName!="")
//				socket->write(QString("/del:" + pMainApplication->delName + "\n").toUtf8());
//			else
//			{
//				pMainApplication->READY_TO_SEND=false;
//				CURRENT_DATA_IS_SENT=false;
//				pMainApplication->ClearSketchNT();
//			}
//		}
//		else if(pMainApplication->m_modeGrip_R==m_markMode)
//		{
//			qDebug()<<"marker position = "<<pMainApplication->markerPOS;
//			socket->write(QString("/marker:" + pMainApplication->markerPOS + "\n").toUtf8());
//		}
//		if(pMainApplication->READY_TO_SEND==true)
//			CURRENT_DATA_IS_SENT=true;
//	}
//
//
//	QTimer::singleShot(20, this, SLOT(RunVRMainloop()));
//}

//


