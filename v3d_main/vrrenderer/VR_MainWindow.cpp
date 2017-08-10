#include "VR_MainWindow.h"

#include "v3dr_gl_vr.h"
#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
VR_MainWindow::VR_MainWindow() :
	QWidget()
{
	//vr_glWidget = 0;
	//vr_glWidget = new VR_GLWidget(this);
   /* if (!vr_glWidget || !(vr_glWidget->isValid()))
    {
    	qDebug("ERROR: Failed to create OpenGL Widget or Context!!! \n");
    }
	else
	{
		qDebug("Successed to create OpenGL Widget");
	}
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(vr_glWidget);
	setLayout(mainLayout);*/

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
        QString userNameDefault = "";
        if(!settings.value("vr_userName").toString().isEmpty())
            userNameDefault = settings.value("vr_userName").toString();
        bool ok2;
        userName = QInputDialog::getText(0, "Lgoin Name",
                                             "Please enter your login name:", QLineEdit::Normal,
                                             userNameDefault, &ok2);
        if(!ok2 || userName.isEmpty())
        {
            qDebug()<<"WRONG!EMPTY! ";
            return SendLoginRequest();
        }else
            settings.setValue("vr_userName", userName);
    }
    else
    {
        qDebug()<<"WRONG!EMPTY! ";
        return SendLoginRequest();
    }

    /*qDebug()<<"Please enter the server address: ";
	std::string str;
	std::cin>>str;
	QString serverName = QString::fromStdString(str);
    if (serverName.isEmpty()) {
		qDebug()<<"WRONG!EMPTY! ";
        return SendLoginRequest();
    }

	qDebug()<<"Please enter your login name: ";
	str="";
	std::cin>>str;
	userName = QString::fromStdString(str);
    if (userName.isEmpty()) {
		qDebug()<<"WRONG!EMPTY! ";
        return SendLoginRequest();
    }*/

    socket->connectToHost(serverName, PORT);
	if(!socket->waitForConnected(15000))
	{
		if(socket->state()==QAbstractSocket::UnconnectedState)
		{
			qDebug()<<"Cannot connect with Server. Unknown error.";
			return 0;
		}	
	}
	return 1;
}

void VR_MainWindow::onReadySend(QString &send_MSG) {
	//qDebug()<<"Please input the message you want to send.";

	//std::string str;
	//std::getline (std::cin,str);
	//QString message = QString::fromStdString(str);
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
		qDebug()<<"The message is empty! Retry!";
		//on_pbSend_clicked();
	}
}

void VR_MainWindow::onReadyRead() {
    QRegExp usersRex("^/users:(.*)$");
    QRegExp systemRex("^/system:(.*)$");
    QRegExp messageRex("^(.*):(.*)$");

    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();

        if (usersRex.indexIn(line) != -1) {
            QStringList users = usersRex.cap(1).split(",");
			qDebug()<<"Current users are:";
            foreach (QString user, users) {
                qDebug()<<user;
            }
        }
        else if (systemRex.indexIn(line) != -1) {
            QString msg = systemRex.cap(1);
			qDebug()<<"System's Broadcast:"<<msg;

        }

        else if (messageRex.indexIn(line) != -1) {
            QString user = messageRex.cap(1);
            QString message = messageRex.cap(2);
			qDebug()<<"user, "<<user<<" said: "<<message;
			/*if(user!=userName){
				//on_pbSend_clicked();       
			}
			if(QString::compare(message,"change")==0)
			{
				pMainApplication->m_bShowMorphologyLine = !pMainApplication->m_bShowMorphologyLine;
				pMainApplication->m_bShowMorphologySurface = !pMainApplication->m_bShowMorphologySurface;
			}*/
			if(pMainApplication)
			{
				if(user==userName)
				{
					pMainApplication->READY_TO_SEND=false;
					CURRENT_DATA_IS_SENT=false;
					pMainApplication->ClearSketchNT();
				}
				pMainApplication->UpdateRemoteNT(message);

			}
		}
    }
}

void VR_MainWindow::onConnected() {


    socket->write(QString("/login:" +userName + "\n").toUtf8());

}

void VR_MainWindow::onDisconnected() {
    qDebug("Now disconnect with the server."); 
	this->close();

}
void VR_MainWindow::StartVRScene(NeuronTree nt, My4DImage *i4d, MainWindow *pmain) {
	
	pMainApplication = new CMainApplication( 0, 0 );
	//pMainApplication->setnetworkmodetrue();//->NetworkModeOn=true;
	pMainApplication->mainwindow =pmain; 
    pMainApplication->loadedNT.listNeuron.clear();
    pMainApplication->loadedNT.hashNeuron.clear();
    pMainApplication->loadedNT.listNeuron = nt.listNeuron;
    pMainApplication->loadedNT.hashNeuron = nt.hashNeuron;
	if(i4d->valid())
	{
		pMainApplication->img4d = i4d;
		pMainApplication->m_bHasImage4D=true;
	}

    //pMainApplication->img4d = img4d;

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return;
	}

	//pMainApplication->RunMainLoop();
	RunVRMainloop();
}

void VR_MainWindow::RunVRMainloop()
{
	//qDebug()<<"run here 4";
	//qApp->processEvents();
	bool bQuit=pMainApplication->HandleOneIteration();
	if(bQuit==true)
	{
		qDebug()<<"Now quit VR";
		socket->disconnectFromHost();
		delete pMainApplication;
		pMainApplication=0;
		return;
	}
	if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
	//READY_TO_SEND is set to true by the "trigger button up" event;
	//client sends data to server (using onReadySend());
	//server sends the same data back to client;
	//READY_TO_SEND is set to false in onReadyRead();
	//CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
	{
		onReadySend(pMainApplication->NT2QString(pMainApplication->sketchNT));
		CURRENT_DATA_IS_SENT=true;
	}
	QTimer::singleShot(20, this, SLOT(RunVRMainloop()));
	
}


//-----------------------------------------------------------------------------
// Purpose: for standalone VR.
//-----------------------------------------------------------------------------
bool doimageVRViewer(NeuronTree nt, My4DImage *i4d, MainWindow *pmain)
{


	CMainApplication *pMainApplication = new CMainApplication( 0, 0 );
	//pMainApplication->setnetworkmodefalse();//->NetworkModeOn=false;
    pMainApplication->mainwindow = pmain;
    
    pMainApplication->loadedNT.listNeuron.clear();
    pMainApplication->loadedNT.hashNeuron.clear();
    pMainApplication->loadedNT.listNeuron = nt.listNeuron;
    pMainApplication->loadedNT.hashNeuron = nt.hashNeuron;
	if(i4d->valid())
	{
		pMainApplication->img4d = i4d;
		pMainApplication->m_bHasImage4D=true;
	}
	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	return 0;
}
