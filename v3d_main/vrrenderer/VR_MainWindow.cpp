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
	VR_Communicator = TeraflyCommunicator;
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
//    connect(this,SIGNAL(sendPoolHead()),this,SLOT(onReadySendSeg()));
    userName = TeraflyCommunicator->userId;
    qDebug()<<"userName "<<userName<<" "<<VR_Communicator->userId;
    CURRENT_DATA_IS_SENT=false;
}

VR_MainWindow::~VR_MainWindow() {
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
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
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
            if(listwithheader.size()<1) return;

            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            int type=-1;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
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
            }

            if(user==VR_Communicator->userId)
            {
                qDebug()<<"release lock";
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug()<<"user = "<<user<<" "<<userName;
            }
        }else if(operationtype == "delline")
        {
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
    //        qDebug()<<"list with header:"<<listwithheader;
            if(listwithheader.size()<1) return;
             QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            qDebug()<<"user="<<user;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);

                }

                if(pMainApplication&&!coords.isEmpty())
                {
                    if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                    {
                        qDebug()<<"delete in block failed";
                            listwithheader.removeAt(0);
                            VR_Communicator->emitDelSeg(listwithheader.join(";"), 0);
                            //全局删线处理
                    }
                }
            }


            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;

            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userName;
            }
        }else if(operationtype == "addmarker")
        {
            qDebug()<<"TeraVR add marker";
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                auto node=listwithheader[1].split(" ");
                int type=node[0].toInt();
                float mx = node.at(1).toFloat();
                float my = node.at(2).toFloat();
                float mz = node.at(3).toFloat();
                XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, type);
                //需要判断点是否在图像中，如果不在则全局处理
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userName;
            }
        }else if(operationtype == "delmarker")
        {
            qDebug()<<"TeraVR del marker";
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                auto node=listwithheader[1].split(" ");
                float mx = node.at(1).toFloat();
                float my = node.at(2).toFloat();
                float mz = node.at(3).toFloat();
                XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                if(pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z))
                {
                    listwithheader.removeAt(0);
                    VR_Communicator->emitDelMarker(listwithheader.join(";"));
                }
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userName;
            }
        }else if(operationtype == "retypeline")
        {
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();
            int type=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[2].trimmed().toInt();

            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    coords.push_front(ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat()));
                }
                if(pMainApplication&&!coords.isEmpty())
                  {

                      if(!pMainApplication->retypeSegment(
                                  coords,
                                  1.0*VRVolumeCurrentRes.x/VRvolumeMaxRes.x,
                                  type))
                      {
                          qDebug()<<"Vr call fly retype ";
                          listwithheader.removeAt(0);
                          VR_Communicator->emitRetypeSeg(listwithheader.join(";"),type,0);
                      }
                  }
            }

              if(user==VR_Communicator->userId)
              {
                  pMainApplication->READY_TO_SEND=false;
                  CURRENT_DATA_IS_SENT=false;
              }else{
                  qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userName;
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
	while(!bQuit)
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
                    if(VR_Communicator&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
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
                        result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                        .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&
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
                    QString ConvertedmarkerPOS = ConvertToMaxGlobal(pMainApplication->markerPosTobeDeleted);
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    result.push_back(ConvertedmarkerPOS);
                    if(ConvertedmarkerPOS.split(" ")[0]=="-1")
                    {
                        qDebug()<<"TeraVR del marker";
                        if(VR_Communicator&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            VR_Communicator->UpdateDelMarkerSeg(QString(result.join(",")));
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
                        result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                        .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&
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
                        result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                        .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    qDebug()<<"result = "<<result;
                    QStringList waitsends;
                    for(auto nt:pMainApplication->segaftersplit)
                    {
                        QStringList waitsend=pMainApplication->NT2QString(nt);
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                        waitsends.push_back(waitsend.join(","));
                    }
                    pMainApplication->segaftersplit.clear();
                    qDebug()<<"waitsends = "<<waitsends;


                    if(VR_Communicator&&
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

QString VR_MainWindow::ConvertToMaxGlobal(QString coords)
{
	float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
	float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
	float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
    int type=coords.section(' ',3, 3).toInt();
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
