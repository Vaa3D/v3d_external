#include "managesocket.h"
#include <QDataStream>
#include <QFile>
#include <QCoreApplication>
#include <QRegExp>
#include <QTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include "V3dR_Communicator.h"
#include "../../terafly/src/control/CImport.h"

ManageSocket::ManageSocket(QObject *parent):QTcpSocket(parent)
{
    resetDataType();
    filepaths.clear();
    connect(this,SIGNAL(readyRead()),this,SLOT(onreadyRead()));
//    connect(this,SIGNAL(disconnected()),this,SLOT(ondisconnect()));
}
void ManageSocket::onreadyRead()
{
    if(!datatype.isFile)
    {
        if(canReadLine())
        {
            QString msg=readLine();
            if(!msg.endsWith('\n'))
            {
                disconnectFromHost();
                this->deleteLater();
            }else
            {
                msg=msg.trimmed();
                QRegExp reg("FILENAMESIZE:(.*)");
                if(reg.indexIn(msg)!=-1)
                {
                    datatype.isFile=true;
                    auto fileNameAndSize=reg.cap(1).split("*;*");
                    if(fileNameAndSize.size()!=2)
                    {
                        disconnectFromHost();
                        this->deleteLater();
                    }
                    datatype.filename=fileNameAndSize[0];
                    datatype.filesize=fileNameAndSize[1].toLongLong();
                }else{
                    QStringList list;
                    list.push_back("00"+msg);
                    processReaded(list);
                }
                onreadyRead();
            }
        }
    }
    else{
        if(bytesAvailable()>=datatype.filesize)
        {
            if(!QDir(QCoreApplication::applicationDirPath()+"/download").exists())
                QDir(QCoreApplication::applicationDirPath()).mkdir("download");
            QString filePath=QCoreApplication::applicationDirPath()+"/download/"+datatype.filename;
            QFile file(filePath);
            file.open(QIODevice::WriteOnly);
            int length=file.write(read(datatype.filesize));
            if(length!=datatype.filesize)
            {
                qDebug()<<"Error:read file";
            }
            file.flush();
            file.close();
            QStringList list;
            list.push_back("11"+filePath);
            resetDataType();
            processReaded(list);
            onreadyRead();
        }
    }
}

void ManageSocket::resetDataType()
{
    datatype.isFile=false;
    datatype.filesize=0;
    datatype.filename.clear();
}

void ManageSocket::sendMsg(QString type,QString msg)
{
    QString data=type+":"+msg+"\n";
    int length=write(data.toStdString().c_str(),data.size());
    if(data.size()!=length)
        qDebug()<<"Error:send "+data;
    else
        qDebug()<<data.toStdString().c_str();
    flush();
}

void ManageSocket::sendFiles(QStringList filePathList,QStringList fileNameList)
{
    for(int i=0;i<filePathList.size();i++)
    {
        QFile f(filePathList[i]);
        if(!f.open(QIODevice::ReadOnly))
        {
            qDebug()<<"Manage:cannot open file "<<fileNameList[i]<<" "<<f.errorString();
            return ;
        }
        QByteArray fileData=f.readAll();
        sendMsg("FILENAMESIZE",(fileNameList[i]+"*;*"+QString::number(fileData.size())));
        int length=write(fileData);
        if(length!=fileData.size())
        {
            qDebug()<<"Error:send data";
        }
        flush();
    }
    for(auto filepath:filePathList)
    {
        if(filepath.contains("/tmp/"))
        {
            QFile(filepath).remove();
        }
    }
}

void ManageSocket::processReaded(QStringList list)
{
    for(auto msg:list)
    {
        if(msg.startsWith("00"))
        {
            processMsg(msg.remove(0,2));
        }
    }
}
void ManageSocket::processMsg( QString &msg)
{

    QRegExp FileList("CurrentFiles:(.*)");//down;data:CurrentFiles
    QRegExp CommunPort("Port:(.*)");
    if(FileList.indexIn(msg)!=-1)
    {
        QStringList response=FileList.cap(1).trimmed().split(";");
        if(response.size()<2)
        {
            qDebug()<<"error:msg is err";
            return;
        }
        QString type=response.at(0);
        listwidget=new QListWidget;
        listwidget->resize(300,500);
        listwidget->setWindowTitle("choose annotation file");
        listwidget->clear();

        if(type=="down")
        {
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                    this,SLOT(download(QListWidgetItem*)));
            for(int i=2;i<response.size();i++)
            {
                listwidget->addItem(response.at(i));
            }
        }
        else if(type=="load")
        {
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                    this,SLOT(load(QListWidgetItem*)));
            for(int i=2;i<response.size();i++)
            {
                if(response.at(i).endsWith("ano"))
                    listwidget->addItem(response.at(i));
            }
        }
        listwidget->show();
    }else if(CommunPort.indexIn(msg)!=-1)
    {
        int port=CommunPort.cap(1).toInt();
        pmain->Communicator = new V3dR_Communicator;
        
        connect(pmain->Communicator,SIGNAL(load(QString)),pmain,SLOT(ColLoadANO(QString)));
        terafly::CViewer *cur_win = terafly::CViewer::getCurrent();
		cur_win->getGLWidget()->TeraflyCommunicator = pmain->Communicator;
        pmain->Communicator->userName=name;

        connect(cur_win->getGLWidget()->TeraflyCommunicator->socket,SIGNAL(connected()),
                this,SLOT(onMessageConnect()));
        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addSeg(QString)),
                cur_win->getGLWidget(),SLOT(CollaAddSeg(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delSeg(QString)),
                cur_win->getGLWidget(),SLOT(CollaDelSeg(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addMarker(QString)),
                cur_win->getGLWidget(),SLOT(CollaAddMarker(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delMarker(QString)),
                cur_win->getGLWidget(),SLOT(CollaDelMarker(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(retypeSeg(QString,int)),
                cur_win->getGLWidget(),SLOT(CollretypeSeg(QString,int)));

        connect(pmain->Communicator->socket,SIGNAL(disconnected()),pmain,SLOT(onMessageDisConnect()));
        pmain->Communicator->socket->connectToHost(ip,port);

        connect(pmain->Communicator,SIGNAL(updateuserview(QString)),pmain,SLOT(updateuserview(QString)));

        if(!pmain->Communicator->socket->waitForConnected())
        {
            QMessageBox::information(0,tr("Message "),
                             tr("connect failed"),
                             QMessageBox::Ok);
            return;
        }

//            connect(this,SIGNAL(signal_communicator_read_res(QString,XYZ*)),
//                    cur_win->getGLWidget()->TeraflyCommunicator,SLOT(read_autotrace(QString,XYZ*)));//autotrace
    }
}

void ManageSocket::download(QListWidgetItem* item)
{
    QString filename=item->text().trimmed();
    if(filename.endsWith(".ano"))
    {
//        sendMsg(filename+";"+filename+".apo;"+filename+".eswc"+":Download");
        sendMsg("Download",filename+";"+filename+".apo;"+filename+".eswc");
    }
    else
    {
        sendMsg("Download",filename);
    }
    listwidget->deleteLater();
    listwidget=nullptr;
    qDebug()<<"delete lsitwidget";
}

void ManageSocket::load(QListWidgetItem* item)
{
    QString filename=item->text().trimmed();
    if(filename.endsWith(".ano"))
    {
        sendMsg("LoadANO",filename.left(filename.size()-4));
    }
    else
        qDebug()<<"choose file with .ano";
    listwidget->deleteLater();
    listwidget=nullptr;
    qDebug()<<"delete lsitwidget";
}

void ManageSocket::onMessageConnect()
{
    int maxresindex = terafly::CImport::instance()->getResolutions()-1;
    IconImageManager::VirtualVolume* vol = terafly::CImport::instance()->getVolume(maxresindex);
    pmain->Communicator->ImageMaxRes = XYZ(vol->getDIM_H(),vol->getDIM_V(),vol->getDIM_D());
    pmain->teraflyVRView->setDisabled(false);
    pmain->collaborationVRView->setEnabled(true);
    pmain->collautotrace->setEnabled(false);
    QMessageBox::information(0,tr("Message "),
                     tr("Load Annotation Sucess!"),
                     QMessageBox::Ok);
}




