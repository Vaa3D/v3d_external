//#include <windows.h>
#include "v3dr_gl_vr.h"
#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"

#include <QtGui>
#include <QRegExp>
//#include <QMessageBox>
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

    if (!send_MSG.isEmpty()) {
        if((send_MSG!="exit")&&(send_MSG!="quit"))
        {

            socket->write(QString(send_MSG+"\n").toUtf8());
            qDebug()<<"send:"<<send_MSG;
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
	for(int i=0;(i<seg.row.size());i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
	{
		V_NeuronSWC_unit curSWCunit = seg.row[i];
		char packetbuff[300];

		
		sprintf(packetbuff,"%5.3f %5.3f %5.3f",curSWCunit.x,curSWCunit.y,curSWCunit.z);
		messageBuff +=packetbuff;
	}

	QString str=QString::fromStdString(messageBuff);
	return str;
}

void V3dR_Communicator::MsgToV_NeuronSWC(QString msg)
{
    QStringList qsl = QString(msg).trimmed().split(" ",Qt::SkipEmptyParts);
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


