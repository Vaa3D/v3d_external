#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>




FileSocket_send::FileSocket_send(QString ip,QString port,QString anofile_path,QObject *parent)
    :QTcpSocket (parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(readMSG()));
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    this->connectToHost(ip,port.toInt());
    QRegExp pathRex("(.*).ano");
    if(pathRex.indexIn(anofile_path)!=-1)
    {
        anopath=pathRex.cap(1);

        QFileInfo  anofile_info(anopath+".ano");
        anoname=anofile_info.fileName();
        QRegExp anoRex("(.*).ano");
        if(anoRex.indexIn(anoname)!=-1)
            anoname=anoRex.cap(1);
    }
    sendFile(anopath+".ano",anoname+".ano");
}
void FileSocket_send::sendFile(QString filepath, QString filename)
{
    QFile f(filepath);
    f.open(QIODevice::ReadOnly);
    QByteArray data=f.readAll();
    QByteArray block;

    QDataStream dts(&block,QIODevice::WriteOnly);
    dts.setVersion(QDataStream::Qt_4_7);

    dts<<qint64(0)<<qint64(0)<<filename;
    dts.device()->seek(0);
    dts<<(qint64)(block.size()+f.size());
    dts<<(qint64)(block.size()-sizeof(qint64)*2);
    dts<<filename;
    dts<<data;

    this->write(block);
    qDebug()<<filepath<<"++";
}
void FileSocket_send::readMSG()
{
    while (this->canReadLine()) {

        QRegExp anoRex("received (.*).ano\n");
        QRegExp swcRex("received (.*).eswc\n");
        QRegExp apoRex("received (.*).apo\n");
        QString MSG=QString::fromUtf8(this->readLine());
        qDebug()<<"fileMSG:"<<MSG;
        if(anoRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.eswc",anoname+".ano.eswc");
        }else if(swcRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.apo",anoname+".ano.apo");
        }else if(apoRex.indexIn(MSG)!=-1)
        {
            qDebug()<<"file upload is ok.";
            this->disconnectFromHost();
            QMessageBox::information(0, tr("information"),tr("import successfully."));
        }
    }
}
ManageSocket::ManageSocket(QObject *parent):QTcpSocket (parent)
{
    MSGsocket=0;FileRec=0;
    messageport.clear();
    loadfilename.clear();
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
}

void ManageSocket::onReadyRead()
{
    QRegExp LoginRex("(.*):log in success.\n");
    QRegExp LogoutRex("(.*):log out success.\n");
    QRegExp ImportRex("(.*):import port.\n");
    QRegExp CurrentDirDownExp("(.*):currentDir_down.\n");
    QRegExp CurrentDirLoadExp("(.*):currentDir_load.\n");
    QRegExp MessagePortExp("(.*):messageport.\n");

	if(this->canReadLine())
	{
		QString manageMsg=QString::fromUtf8(this->readLine());
		qDebug()<<"receive:"<<manageMsg;
		if(LoginRex.indexIn(manageMsg)!=-1)
		{
			QMessageBox::information(0, tr("information"),tr("login successfully."));
		}else if (LogoutRex.indexIn(manageMsg)!=-1)
		{
            qDebug()<<"test in logout";
			this->disconnectFromHost();
		}else if(ImportRex.indexIn(manageMsg)!=-1)
		{
            if(this->state()==QAbstractSocket::UnconnectedState)
            {
                QMessageBox::information(0, tr("Error"),tr("can not connect with Manageserver."));
                return ;
            }

            QString anofile_path = QFileDialog::getOpenFileName(0,"标题",".","*.ano");
            qDebug()<<anofile_path;
            FileSocket_send *filesocket_send=new FileSocket_send(ip,ImportRex.cap(1),anofile_path);

        }else if(CurrentDirDownExp.indexIn(manageMsg)!=-1)
		{
            qDebug()<<"1";
            QString currentDir=CurrentDirDownExp.cap(1);
			QStringList file_list=currentDir.split(";");
			QWidget *widget=new QWidget(0);
			widget->setWindowTitle("choose annotation file ");
			QListWidget *filelistWidget=new QListWidget;
			QVBoxLayout mainlayout(widget);

			mainlayout.addWidget(filelistWidget);
			connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,SLOT(send1(QListWidgetItem*)));

			connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
				widget,SLOT(close()));

			filelistWidget->clear();
			for(uint i=0;i<file_list.size();i++)
			{
				QIcon icon("file.png");
				QListWidgetItem *tmp=new QListWidgetItem(icon,file_list.at(i));
				qDebug()<<file_list.at(i);
				filelistWidget->addItem(tmp);
			}
			widget->show();
        }else if(CurrentDirLoadExp.indexIn(manageMsg)!=-1)
        {
            qDebug()<<"2";
            QString currentDir=CurrentDirLoadExp.cap(1);
            QStringList file_list=currentDir.split(";");
            QWidget *widget=new QWidget(0);
            widget->setWindowTitle("choose annotation file ");
            QListWidget *filelistWidget=new QListWidget;
            QVBoxLayout mainlayout(widget);

            mainlayout.addWidget(filelistWidget);
            connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,SLOT(send2(QListWidgetItem*)));

            connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                widget,SLOT(close()));

            filelistWidget->clear();
            for(uint i=0;i<file_list.size();i++)
            {
                QIcon icon("file.png");
                QListWidgetItem *tmp=new QListWidgetItem(icon,file_list.at(i));
                qDebug()<<file_list.at(i);
                filelistWidget->addItem(tmp);
            }
            widget->show();
        }else if(MessagePortExp.indexIn(manageMsg)!=-1)
        {
            messageport=MessagePortExp.cap(1);
            emit makeMessageSocket(ip,MessagePortExp.cap(1),name);
        }

	}
}


void ManageSocket::send1(QListWidgetItem *item)
{
    FileServer *fileserver=new FileServer;
    if(fileserver->listen(QHostAddress::Any,9998))
    {
        qDebug()<<"88888";
        qDebug()<<item->text();
        this->write(QString(item->text()+":choose1."+"\n").toUtf8());
        qDebug()<<QString(QString(item->text()+":choose1."));
    }
}

void ManageSocket::send2(QListWidgetItem *item)
{
    loadfilename.clear();FileRec=0;
        FileServer *fileserver=new FileServer;
        connect(fileserver,SIGNAL(receivedfile(QString)),this,SLOT(receivefile(QString)));
        if(fileserver->listen(QHostAddress::Any,9998))
        {
            qDebug()<<"88888";
            qDebug()<<item->text();
            loadfilename=item->text();
            this->write(QString(item->text()+":choose2."+"\n").toUtf8());
            qDebug()<<QString(QString(item->text()+":choose2."));
        }
}

void ManageSocket::messageMade()
{
    qDebug()<<" in messageMade.()";
    MSGsocket=1;
    if(FileRec==1)
        emit loadANO(loadfilename);
}

void ManageSocket::receivefile(QString anofile)
{
    qDebug()<<" in receivefile().";
    FileRec=1;

    if(MSGsocket==1)
        emit loadANO(loadfilename);
}


V3dR_Communicator::V3dR_Communicator(bool *client_flag /*= 0*/, V_NeuronSWC_list* ntlist/*=0*/)
{
	clienton = client_flag;
	NTList_SendPool = ntlist;
	NTNumReceieved=0;
	NeuronTree  nt = terafly::PluginInterface::getSWC();
	int tempntsize = nt.listNeuron.size();
	cout<<"tempnt size is liqiqqqqq "<<tempntsize<<endl;

	// NTNumcurrentUser = (*ntlist).size();
	// std::cout<<"NTNumcurrentUser "<<NTNumcurrentUser<<std::endl;

	userName="";
	QRegExp regex("^[a-zA-Z]\\w+");
    socket = 0;

//	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
//	connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	CURRENT_DATA_IS_SENT=false;
//    nextblocksize=0;
}

	V3dR_Communicator::~V3dR_Communicator() {

}

bool V3dR_Communicator::SendLoginRequest(QString ip,QString port,QString user) {
    socket=new QTcpSocket;
//    connect(this->managesocket,SIGNAL(disconnected()),socket,SLOT(disconnectFromHost()));
    connect(socket,SIGNAL(connected()),this,SLOT(onConnected()));
 //   connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket,SIGNAL(disconnected()),socket,SLOT(deleteLater()));

    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    qDebug()<<"start login messageserver";
    QSettings settings("HHMI", "Vaa3D");

    userName=user;
//    vr_Port=port;
    Agent agent00={
        //with local information
        userName,
        true,//means this struct point to itself,no need to render
        21,
        0,
    };
    Agents.push_back(agent00);
	
    socket->connectToHost(ip, port.toUInt());
	if(!socket->waitForConnected(15000))
	{
		if(socket->state()==QAbstractSocket::UnconnectedState)
		{
			qDebug()<<"Cannot connect with Server. Unknown error.";
			return 0;
		}	
	}
    qDebug()<<"User:  "<<userName<<".  Connected with server: "<<ip<<" :"<<vr_Port;
    emit messageMade();

	return 1;
}

void V3dR_Communicator::UpdateSendPoolNTList(V_NeuronSWC seg)
{
	NTList_SendPool->append(seg);

    onReadySend(QString("/seg: "+V_NeuronSWCToSendMSG(seg)));
}

void V3dR_Communicator::onReadySend(QString send_MSG) {

    qDebug()<<endl<<send_MSG<<endl;
    if (!send_MSG.isEmpty()) {
        if((send_MSG!="exit")&&(send_MSG!="quit"))
        {

            socket->write(QString(send_MSG+"\n").toUtf8());
            qDebug()<<"111111send:\n"<<send_MSG;

        }
        else
        {

            socket->write(QString("/say: GoodBye~\n").toUtf8());
        }
	}
	else
	{
		qDebug()<<"The message is empty!";
	}
}

void V3dR_Communicator::onReadyRead() {
	QRegExp usersRex("^/users:(.*)$");
	QRegExp systemRex("^/system:(.*)$");
	QRegExp hmdposRex("^/hmdpos:(.*)$");
	QRegExp colorRex("^/color:(.*)$");
	QRegExp deletecurveRex("^/del_curve:(.*)$");
	QRegExp markerRex("^/marker:(.*)$");
	QRegExp delmarkerRex("^/del_marker:(.*)$");
	QRegExp dragnodeRex("^/drag_node:(.*)$");
	QRegExp creatorRex("^/creator:(.*)$");
	QRegExp messageRex("^/seg:(.*)$");


	while (socket->canReadLine()) {
		QString line = QString::fromUtf8(socket->readLine()).trimmed();

		qDebug()<<"receive :"<<line;

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
		else if(colorRex.indexIn(line) != -1) {
			//qDebug()<<"run color";
			//QString colorFromServer = colorRex.cap(1);
			//qDebug()<<"the color receieved is :"<<colorFromServer;
			QStringList clrMSGs = colorRex.cap(1).split(" ");

			if(clrMSGs.size()<2) return;
			QString user=clrMSGs.at(0);
			QString clrtype=clrMSGs.at(1);
			//for(int i=0;i<VR_Communicator->Agents.size();i++)
			//{
			//	if(VR_Communicator->Agents.at(i).name!=user) continue;
			//	//update agent color
			//	VR_Communicator->Agents.at(i).colorType=clrtype.toInt();
			//	qDebug()<<"user:"<<user<<" receievedColorTYPE="<<VR_Communicator->Agents.at(i).colorType;
			//	if(user == userName)
			//		pMainApplication->SetupCurrentUserInformation(userName.toStdString(), VR_Communicator->Agents.at(i).colorType);
			//}
		}
		else if (deletecurveRex.indexIn(line) != -1) {
		//	qDebug() << "------------"<<line;
		//	QStringList delMSGs = deletecurveRex.cap(1).split(" ");
		//	if(delMSGs.size()<2) 
		//	{
		//		qDebug()<<"size < 2";
		//		return;
		//	}
		//	QString user = delMSGs.at(0);
		//	float dx = delMSGs.at(1).toFloat();
		//	float dy = delMSGs.at(2).toFloat();
		//	float dz = delMSGs.at(3).toFloat();
		//	float resx = delMSGs.at(4).toFloat();
		//	float resy = delMSGs.at(5).toFloat();
		//	float resz = delMSGs.at(6).toFloat();

		//	pMainApplication->collaborationTargetdelcurveRes = XYZ(resx,resy,resz);

		//	qDebug()<<"pMainApplication->collaborationTargetdelcurveRes = XYZ(resx,resy,resz);";
		//	qDebug()<<"user, "<<user<<" delete: "<<dx<<dy<<dz;
		//	XYZ  converreceivexyz = ConvertreceiveCoords(dx,dy,dz);
		//	qDebug()<<"user, "<<user<<" Converted Receive curve: "<<converreceivexyz.x<<" "<<converreceivexyz.y<<" "<<converreceivexyz.z;
		//	XYZ TeraflyglobalPos =XYZ(dx ,dy,dz);

		//	dx/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
		//	dy/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
		//	dz/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);

		//	if(TeraflyglobalPos.x<VRVolumeStartPoint.x || 
		//		TeraflyglobalPos.y<VRVolumeStartPoint.y||
		//		TeraflyglobalPos.z<VRVolumeStartPoint.z||
		//		TeraflyglobalPos.x>VRVolumeEndPoint.x||
		//		TeraflyglobalPos.y>VRVolumeEndPoint.y||
		//		TeraflyglobalPos.z>VRVolumeEndPoint.z
		//		)
		//	{
		//		qDebug()<<"push_back test delete point ";
		//		VROutinfo.deletedcurvespos.push_back(TeraflyglobalPos);
		//	}
		//	qDebug()<<"deletedcurvespos"<<dx<<" "<<dy<<" "<<dz;
		//	if(user==userName)
		//	{
		//		pMainApplication->READY_TO_SEND=false;
		//		CURRENT_DATA_IS_SENT=false;
		//		pMainApplication->ClearCurrentNT();
		//	}
		//	QString delID = pMainApplication->FindNearestSegment(glm::vec3(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z));
		//	qDebug()<<"delete ID"<<delID<<"++++++++++++++++++++";
		//	bool delerror = pMainApplication->DeleteSegment(delID);
		//	qDebug()<<".................................";
		//	if(delerror==true)
		//		qDebug()<<"Segment Deleted.";
		//	else
		//		qDebug()<<"Cannot Find the Segment ";
		//	// pMainApplication->MergeNeuronTrees();
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
			/*pMainApplication->CollaborationTargetMarkerRes = XYZ(resx,resy,resz);
			XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
			qDebug()<<"user, "<<user<<" Converted Receive marker: "<<converreceivexyz.x<<" "<<converreceivexyz.y<<" "<<converreceivexyz.z;
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				qDebug()<<"get message CURRENT_DATA_IS_SENT=false;";
				pMainApplication->ClearCurrentNT();
			}*/

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
			/*XYZ  converreceivexyz = ConvertreceiveCoords(mx,my,mz);
			if(user==userName)
			{
				pMainApplication->READY_TO_SEND=false;
				CURRENT_DATA_IS_SENT=false;
				pMainApplication->ClearCurrentNT();
			}

			pMainApplication->RemoveMarkerandSurface(converreceivexyz.x,converreceivexyz.y,converreceivexyz.z,colortype);*/
		}
		//dragnodeRex
		else if (messageRex.indexIn(line) != -1) {
			QStringList MSGs = messageRex.cap(1).split(" ");
			for(int i=0;i<MSGs.size();i++)
			{
				qDebug()<<MSGs.at(i)<<endl;
			}
			QString user = MSGs.at(0);
			QString message;
			for(int i=1;i<MSGs.size();i++)
			{
				message +=MSGs.at(i);
				if(i != MSGs.size()-1)
					message +=" ";


				qDebug()<<MSGs.at(i)<<endl;
			}
			qDebug()<<"user, "<<user<<" said: "<<message;
			
			
		}
	}      
}

void V3dR_Communicator::CollaborationMainloop(){
	Collaborationsendmessage();
	Collaborationaskmessage();
	QTimer::singleShot(200, this, SLOT(CollaborationMainloop()));
}
void V3dR_Communicator::onConnected() {

//    socket->write(QString("/login:" +userName + "\n").toUtf8());
    qDebug()<<"gere is onconnected.";
    onReadySend(QString("/login:" +userName));

}
void V3dR_Communicator::Collaborationsendmessage()
{
//	socket->write(QString("/say: send~\n").toUtf8());
    onReadySend(QString("/say: send~"));//????
}


void V3dR_Communicator::Collaborationaskmessage()
{
        onReadySend(QString("/ask: message"));
}

QString V3dR_Communicator::V_NeuronSWCToSendMSG(V_NeuronSWC seg)
{
	string messageBuff="";
	//add seg extra msg
	messageBuff += seg.name;
	messageBuff+=" ";
	messageBuff += seg.comment;
	messageBuff+=" ";
	messageBuff += seg.file;
//	messageBuff+=" ";
    messageBuff+="_";

	for(int i=0;i<seg.row.size();i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
	{
		V_NeuronSWC_unit curSWCunit = seg.row[i];
		char packetbuff[300];

		
        sprintf(packetbuff,"%ld %d %5.3f %5.3f %5.3f %5.3f %ld %5.3f %5.3f %5.3f %5.3f_",
       curSWCunit.n,curSWCunit.type,curSWCunit.x,curSWCunit.y,curSWCunit.z,
       curSWCunit.r,curSWCunit.parent,curSWCunit.level,curSWCunit.creatmode,curSWCunit.timestamp,
                curSWCunit.tfresindex);

		messageBuff +=packetbuff;
	}



	QString str=QString::fromStdString(messageBuff);
	return str;
}

void V3dR_Communicator::MsgToV_NeuronSWC(QString msg)
{
	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
	int str_size = qsl.size()-(qsl.size()%3);//to make sure that the string list size always be 3*N;
	qDebug()<<"qsl.size()"<<qsl.size()<<"str_size"<<str_size;
	vector<XYZ> loclist_temp;
	XYZ loc_temp;
	for(int i=0;i<str_size;i++)
	{
		qsl[i].truncate(99);
		qDebug()<<qsl[i];
		int iy = i%7;
		if (iy==0)
		{
			loc_temp.x = qsl[i].toFloat();
		}
		else if(iy==1)
		{
			loc_temp.y = qsl[i].toFloat();
		}
		else if(iy==2)
		{
			loc_temp.z = qsl[i].toFloat();
			loclist_temp.emplace_back(loc_temp);
		}

	}
	loc_ReceivePool.push_back(loclist_temp);
	//update AddCurveSWC
}

void V3dR_Communicator::onDisconnected() {
    qDebug("Now disconnect with the server."); 
	*clienton = false;
	//Agents.clear();
	this->close();
    deleteLater();


}


//void V3dR_Communicator::Update3DViewNTList(QString &msg, int type)//may need to be changed to AddtoNTList( , )
//{	
//	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
//	int str_size = qsl.size()-(qsl.size()%7);//to make sure that the string list size always be 7*N;
//	//qDebug()<<"qsl.size()"<<qsl.size()<<"str_size"<<str_size;
//	NeuronSWC S_temp;
//	NeuronTree tempNT;
//	tempNT.listNeuron.clear();
//	tempNT.hashNeuron.clear();
//	//each segment has a unique ID storing as its name
//	tempNT.name  = "sketch_"+ QString("%1").arg(NTNumReceieved++);
//	for(int i=0;i<str_size;i++)
//	{
//		qsl[i].truncate(99);
//		//qDebug()<<qsl[i];
//		int iy = i%7;
//		if (iy==0)
//		{
//			S_temp.n = qsl[i].toInt();
//		}
//		else if (iy==1)
//		{
//			S_temp.type = type;
//		}
//		else if (iy==2)
//		{
//			S_temp.x = qsl[i].toFloat();
//
//		}
//		else if (iy==3)
//		{
//			S_temp.y = qsl[i].toFloat();
//
//		}
//		else if (iy==4)
//		{
//			S_temp.z = qsl[i].toFloat();
//
//		}
//		else if (iy==5)
//		{
//			S_temp.r = qsl[i].toFloat();
//
//		}
//		else if (iy==6)
//		{
//			S_temp.pn = qsl[i].toInt();
//
//			tempNT.listNeuron.append(S_temp);
//			tempNT.hashNeuron.insert(S_temp.n, tempNT.listNeuron.size()-1);
//		}
//	}//*/
//	//RGBA8 tmp= {(unsigned int)type};
//	//tempNT.color = tmp;
//	tempNT.color.i=type;
//	NTList_3Dview->push_back(tempNT);
//	qDebug()<<"receieved nt name is "<<tempNT.name;
//	//updateremoteNT
//}




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


