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
    userName = TeraflyCommunicator->userName;
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
    QRegExp drawlineRex("^/drawline:(.*)$");
    QRegExp dellineRex("^/delline:(.*)$");
    QRegExp addmarkerRex("^/addmarker:(.*)$");
    QRegExp delmarkerRex("^/delmarker:(.*)$");
    QRegExp retypelineRex("^/retypeline:(.*)$");
//	QRegExp dragnodeRex("^/drag_node:(.*)$");
//    QRegExp creatorRex("^/creator:(.*)__(.*)$");
    qDebug()<<"TeraVr reveive:"<<line;
    line=line.trimmed();
    if (drawlineRex.indexIn(line) != -1) {
//        qDebug()<<"TeraVR add seg";
        QStringList listwithheader=drawlineRex.cap(1).split(";",QString::SkipEmptyParts);
        if(listwithheader.size()<1) return;

        QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[0].trimmed();

        int type=-1;
        if(listwithheader.size()>1)
        {
            QVector<XYZ> coords;
//            qDebug()<<"VRVolumeStartPoint="<<VRVolumeStartPoint.x<<" "
//                   <<VRVolumeStartPoint.y<<" "<<VRVolumeStartPoint.z;
//            qDebug()<<"VRVolumeEndPoint="<<VRVolumeEndPoint.x<<" "
//                   <<VRVolumeEndPoint.y<<" "<<VRVolumeEndPoint.z;
//            qDebug()<<"VRVolumeCurrentRes="<<VRVolumeCurrentRes.x<<" "
//                   <<VRVolumeCurrentRes.y<<" "<<VRVolumeCurrentRes.z;
//            qDebug()<<"VRvolumeMaxRes="<<VRvolumeMaxRes.x<<" "
//                   <<VRvolumeMaxRes.y<<" "<<VRvolumeMaxRes.z;
            for(int i=1;i<listwithheader.size();i++)
            {
                auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                coords.push_back(converted);

                type=nodeinfo[0].toInt();
            }
            if(pMainApplication)
            {
                qDebug()<<type<<" "<<coords.size();
                pMainApplication->UpdateNTList(coords,type);
                //需要判断线是否在图像中，如果不在则调用全局处理
            }
        }

        if(user==userName)
        {
            pMainApplication->READY_TO_SEND=false;
            CURRENT_DATA_IS_SENT=false;
        }
    }else if (dellineRex.indexIn(line) != -1) {
        qDebug()<<"TeraVR del seg";
        qDebug()<<line;
        line = dellineRex.cap(1);qDebug()<<line;
        line=line.trimmed();qDebug()<<line;

        QStringList listwithheader=line.split(";",QString::SkipEmptyParts);
//        qDebug()<<"list with header:"<<listwithheader;
        if(listwithheader.size()<1) return;
         QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[0].trimmed();

        qDebug()<<"user="<<user;
        if(listwithheader.size()>1)
        {
            QVector<XYZ> coords;
//            qDebug()<<"VRVolumeStartPoint="<<VRVolumeStartPoint.x<<" "
//                   <<VRVolumeStartPoint.y<<" "<<VRVolumeStartPoint.z;
//            qDebug()<<"VRVolumeEndPoint="<<VRVolumeEndPoint.x<<" "
//                   <<VRVolumeEndPoint.y<<" "<<VRVolumeEndPoint.z;
//            qDebug()<<"VRVolumeCurrentRes="<<VRVolumeCurrentRes.x<<" "
//                   <<VRVolumeCurrentRes.y<<" "<<VRVolumeCurrentRes.z;
//            qDebug()<<"VRvolumeMaxRes="<<VRvolumeMaxRes.x<<" "
//                   <<VRvolumeMaxRes.y<<" "<<VRvolumeMaxRes.z;
            for(int i=1;i<listwithheader.size();i++)
            {
                auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                coords.push_front(converted);
//                qDebug()<<nodeinfo;
//                qDebug()<<converted.x<<" "<<converted.y<<" "<<converted.z;
            }

            if(pMainApplication)
            {
                if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                {
                    qDebug()<<"delete in block failed";
                        listwithheader.removeAt(0);
                        VR_Communicator->emitDelSeg(listwithheader.join(";"));
                        //全局删线处理
                      // auto v3dGL= terafly::CViewer::getCurrent()->getGLWidget();
                       //v3dGL->deleteCurveInMaxRex(listwithheader.join(";"));
                }
            }
        }


        if(user==userName)
        {
            pMainApplication->READY_TO_SEND=false;
            CURRENT_DATA_IS_SENT=false;

        }
    }else if (retypelineRex.indexIn(line)!=-1) {
        qDebug()<<"TeraVR retype seg";
        QStringList listwithheader=retypelineRex.cap(1).split(";",QString::SkipEmptyParts);

        if(listwithheader.size()<1) return;
        QStringList headers=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts);
        qDebug()<<headers;
        QString user=headers[0].trimmed();
        int type=headers[2].trimmed().toInt();

        qDebug()<<"VR new type = "<<type;
        if(listwithheader.size()>1)
        {
            QVector<XYZ> coords;
//            qDebug()<<"VRVolumeStartPoint="<<VRVolumeStartPoint.x<<" "
//                            <<VRVolumeStartPoint.y<<" "<<VRVolumeStartPoint.z;
//                     qDebug()<<"VRVolumeEndPoint="<<VRVolumeEndPoint.x<<" "
//                            <<VRVolumeEndPoint.y<<" "<<VRVolumeEndPoint.z;
//                     qDebug()<<"VRVolumeCurrentRes="<<VRVolumeCurrentRes.x<<" "
//                            <<VRVolumeCurrentRes.y<<" "<<VRVolumeCurrentRes.z;
//                     qDebug()<<"VRvolumeMaxRes="<<VRvolumeMaxRes.x<<" "
//                            <<VRvolumeMaxRes.y<<" "<<VRvolumeMaxRes.z;
            for(int i=1;i<listwithheader.size();i++)
            {
                auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                coords.push_front(ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat()));
            }
            if(pMainApplication)
              {
                  if(!pMainApplication->retypeSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x,type));
                  {
                      listwithheader.removeAt(0);
                      VR_Communicator->emitRetypeSeg(listwithheader.join(";"),type);
                  }
              }
        }

          if(user==userName)
          {
              pMainApplication->READY_TO_SEND=false;
              CURRENT_DATA_IS_SENT=false;
          }
     }else if (addmarkerRex.indexIn(line) != -1) {
        qDebug()<<"TeraVR add marker";
        QStringList listwithheader=addmarkerRex.cap(1).split(";",QString::SkipEmptyParts);

        if(listwithheader.size()<1) return;
        QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[0].trimmed();

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
        if(user==userName)
        {
            pMainApplication->READY_TO_SEND=false;
            CURRENT_DATA_IS_SENT=false;
        }
   }else if (delmarkerRex.indexIn(line) != -1) {
        qDebug()<<"TeraVR del marker";
        QStringList listwithheader=delmarkerRex.cap(1).split(";",QString::SkipEmptyParts);

        if(listwithheader.size()<1) return;
        QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[0].trimmed();

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
        if(user==userName)
        {
            pMainApplication->READY_TO_SEND=false;
            CURRENT_DATA_IS_SENT=false;
        }
    }
/*        if (usersRex.indexIn(line) != -1) {
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
        else*/ /*if (creatorRex.indexIn(line) != -1) {
			QString user = creatorRex.cap(1);
			QStringList markerMSGs = creatorRex.cap(2).split(" ");
            qDebug()<<user<<" "<<userName;
            if (markerMSGs.size() < 7)
			{
                qDebug() << "size < 7";
                if(user.left(userName.count())==userName)
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
                qDebug()<<user<<" "<<userName;
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
        else */
        /*if (dragnodeRex.indexIn(line) != -1) {
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
            if(user.left(userName.count())==userName)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
                pMainApplication->ClearCurrentNT();
            }
            pMainApplication->UpdateDragNodeinNTList(ntnum,swcnum,converreceivexyz.x,converreceivexyz.y,converreceivexyz.z);
        }

}
*/
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

	bQuit=pMainApplication->HandleOneIteration();

    if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
	//READY_TO_SEND is set to true by the "trigger button up" event;
	//client sends data to server (using onReadySend());
	//server sends the same data back to client;
	//READY_TO_SEND is set to false in onReadyRead();
    //CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
	{
        if(pMainApplication->m_modeGrip_R==m_drawMode)
		{
            qDebug()<<"TeraVR add seg";
            QStringList waitsend=pMainApplication->NT2QString(pMainApplication->currentNT);
            waitsend.push_front(QString("%1 TeraVR %2 %3 %4").arg(userName).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
			pMainApplication->ClearCurrentNT();
            if(VR_Communicator&&
                VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
            {
                VR_Communicator->UpdateAddSegMsg(waitsend.join(";"));
                CURRENT_DATA_IS_SENT=true;
            }
		}
		else if(pMainApplication->m_modeGrip_R==m_deleteMode)
		{
            if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
			{
                QStringList result;
                result.push_back(QString("%1 TeraVR %2 %3 %4").arg(VR_Communicator->userName).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                {
                    result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                    .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                }
                if(VR_Communicator&&
                    VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                {
                VR_Communicator->UpdateDelSegMsg(QString(result.join(";")));
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
            qDebug() << "markerPos：" << pMainApplication->markerPosTobeDeleted;
            if(pMainApplication->markerPosTobeDeleted!="")
            {
                QStringList result;
                QString ConvertedmarkerPOS = ConvertToMaxGlobal(pMainApplication->markerPosTobeDeleted);
                result.push_back(QString("%1 TeraVR %2 %3 %4").arg(VR_Communicator->userName).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                result.push_back(ConvertedmarkerPOS);
                if(ConvertedmarkerPOS.split(" ")[0]=="-1")
                {
                    qDebug()<<"TeraVR del marker";
                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateDelMarkerSeg(QString(result.join(";")));
                    }
                }else
                {
                    qDebug()<<"TeraVR add marker";
                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateAddMarkerMsg(QString(result.join(";") ));
                    }
                }
                pMainApplication->markerPosTobeDeleted.clear();
                CURRENT_DATA_IS_SENT=true;
            }
        }else if(pMainApplication->m_modeGrip_R==m_retypeMode)
        {
            if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
            {
                QStringList result;
                result.push_back(QString("%1 TeraVR %2 %3 %4 %5").arg(VR_Communicator->userName).arg(pMainApplication->m_curMarkerColorType).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                {
                    result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                    .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                }
                if(VR_Communicator&&
                    VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                {
                VR_Communicator->UpdateRetypeSegMsg(QString(result.join(";")));
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
                result.push_back(QString("%1 TeraVR %2 %3 %4").arg(VR_Communicator->userName).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                {
                    result.push_back(ConvertToMaxGlobal(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                    .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                }

                QStringList waitsends;
                for(auto nt:pMainApplication->segaftersplit)
                {
                    QStringList waitsend=pMainApplication->NT2QString(nt);
                    waitsend.push_front(QString("%1 TeraVR %2 %3 %4").arg(userName).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    waitsends.push_back(waitsend.join(";"));
                }
                pMainApplication->segaftersplit.clear();


                if(VR_Communicator&&
                    VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                {
//                    VR_Communicator->UpdateDelSegMsg(QString(result.join(";")));
//                    for(auto addmsg:waitsends)
//                    {
//                        VR_Communicator->UpdateAddSegMsg(addmsg);
//                    }
                    //
                    VR_Communicator->UpdateSplitSegMsg(QString(result.join(";")),waitsends.at(0),waitsends.at(2));
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
        }else if(pMainApplication->undo)
        {
            if(VR_Communicator->undoDeque.size())
            {
                VR_Communicator->UpdateUndoDeque();
                CURRENT_DATA_IS_SENT=true;
            }
            else{
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
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }

            pMainApplication->redo=false;
        }

//        else if(pMainApplication->m_modeGrip_R==m_dragMode)
//        {
//            QString ConverteddragnodePOS = ConvertToMaxGlobal(pMainApplication->dragnodePOS);
//            VR_Communicator->sendMsg(QString("/drag_node:" + ConverteddragnodePOS ));
//            CURRENT_DATA_IS_SENT=true;
//        }

//        else if (pMainApplication->m_modeGrip_R == _MovetoCreator)
//        {
//            QString ConvertedmarkerPOS = ConvertToMaxGlobal(pMainApplication->markerPOS);

//            QString QSCurrentRes = QString("%1 %2 %3").arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z);
//            QString QCmainResIndex = QString("%1").arg(pMainApplication->CmainResIndex);
//            VR_Communicator->sendMsg(QString("/creator:" + ConvertedmarkerPOS + " " + QSCurrentRes + " " + QCmainResIndex));
//            qDebug()<<QString("/creator:" + ConvertedmarkerPOS + " " + QSCurrentRes + " " + QCmainResIndex);
//            CURRENT_DATA_IS_SENT = true;
//        }
    }

//        if((pMainApplication->undo==true)&&(pMainApplication->READY_TO_SEND==true)/*&&(CURRENT_DATA_IS_SENT==false)*/)
//        {
//            qDebug()<<"---------undo TV------------";
//            //VR_Communicator->undo();
//            CURRENT_DATA_IS_SENT=true;
//            pMainApplication->undo=false;
//        }
	}

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
