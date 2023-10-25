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
    resetDataInfo();
    filepaths.clear();
    connect(this,SIGNAL(readyRead()),this,SLOT(onreadyRead()));
//    connect(this,SIGNAL(disconnected()),this,SLOT(ondisconnect()));
}
void ManageSocket::onreadyRead()
{
    QDataStream in(this);
    if(dataInfo.dataSize==0)
    {
		if (this->bytesAvailable() >= sizeof(qint32))
        {
            in>>dataInfo.dataSize;
            dataInfo.dataReadedSize+=sizeof (qint32);
        }
        else return;
    }

    if(dataInfo.stringOrFilenameSize==0&&dataInfo.filedataSize==0)
    {
		if (this->bytesAvailable() >= 2 * sizeof(qint32))
        {
            in>>dataInfo.stringOrFilenameSize>>dataInfo.filedataSize;
            dataInfo.dataReadedSize+=(2*sizeof (qint32));
        }else
            return;
    }
    QStringList list;
	if (this->bytesAvailable() >= dataInfo.stringOrFilenameSize + dataInfo.filedataSize)
    {
        qDebug()<<"in down file1";
		QString messageOrFileName = QString::fromUtf8(this->read(dataInfo.stringOrFilenameSize), dataInfo.stringOrFilenameSize);


        qDebug()<<messageOrFileName<<dataInfo.filedataSize;
        if(dataInfo.filedataSize)
        {
            if(!QDir(QCoreApplication::applicationDirPath()+"/download").exists())
            {
                QDir(QCoreApplication::applicationDirPath()).mkdir("download");
            }
            qDebug()<<"in read file";
            QString filePath=QCoreApplication::applicationDirPath()+"/download/"+messageOrFileName;
            qDebug()<<filePath;
            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(this->read(dataInfo.filedataSize)); file.flush();
                file.close();
            }else
            {
                qDebug()<<filepaths<<" "<<file.error();
            }
            qDebug()<<"in down file2";
        }else
        {
            list.push_back("00"+messageOrFileName);
        }
        dataInfo.dataReadedSize+=(dataInfo.stringOrFilenameSize+dataInfo.filedataSize);
        dataInfo.stringOrFilenameSize=0;
        dataInfo.filedataSize=0;
        if(dataInfo.dataReadedSize==dataInfo.dataSize)
            resetDataInfo();
        processReaded(list);
    }else
        return;
    onreadyRead();
}

void ManageSocket::resetDataInfo()
{
     dataInfo.dataSize=0;dataInfo.stringOrFilenameSize=0;
     dataInfo.dataReadedSize=0;dataInfo.filedataSize=0;
}

void ManageSocket::sendMsg(QString msg)
{
    qint32 stringSize=msg.toUtf8().size();
    qint32 totalsize=3*sizeof (qint32)+stringSize;
    QByteArray block;
    QDataStream dts(&block,QIODevice::WriteOnly);
    dts<<qint32(totalsize)<<qint32(stringSize)<<qint32(0);
    block+=msg.toUtf8();
    this->write(block);
    this->flush();
    qDebug()<<msg;
}

void ManageSocket::sendFiles(QStringList filePathList,QStringList fileNameList)
{
    int totalsize=sizeof(qint32);
    QList<QByteArray> blocks;
    for(int i=0;i<filePathList.size();i++)
    {
        QByteArray block;
        block.clear();
        QDataStream dts(&block,QIODevice::WriteOnly);
        QFile f(filePathList[i]);
        if(!f.open(QIODevice::ReadOnly))
            qDebug()<<"cannot open file "<<fileNameList[i]<<" "<<f.errorString();
        QByteArray fileName=fileNameList[i].toUtf8();
        QByteArray fileData=f.readAll();
        f.close();
        dts<<qint32(fileName.size())<<qint32(fileData.size());
        block=block+fileName;
        block=block+fileData;
        blocks.push_back(block);
        totalsize+=block.size();
    }
    QByteArray block;

    block.clear();
    QDataStream dts(&block,QIODevice::WriteOnly);
    dts<<qint32(totalsize);
    for(int i=0;i<blocks.size();i++)
        block=block+blocks[i];
    qDebug()<<totalsize<<' '<<block.size();
    this->write(block);
    this->flush();

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

    QRegExp FileList("(.*):CurrentFiles");//down;data:CurrentFiles
    QRegExp CommunPort("(.*):Port");
    if(FileList.indexIn(msg)!=-1)
    {
        QStringList response=FileList.cap(1).trimmed().split(";");
        if(response.size()<2)
        {
            qDebug()<<"error:msg is err";
            return;
        }
        QString type=response.at(response.size()-2);
        QString dirname=response.at(response.size()-1);
        listwidget=new QListWidget;
        listwidget->resize(300,500);
        listwidget->setWindowTitle("choose annotation file");
        listwidget->clear();

        if(type=="down")
        {
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                    this,SLOT(download(QListWidgetItem*)));
            for(int i=0;i<response.size()-2;i++)
            {
                listwidget->addItem(response.at(i));
            }
        }
        else if(type=="load")
        {
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                    this,SLOT(load(QListWidgetItem*)));
            for(int i=0;i<response.size()-2;i++)
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
        pmain->Communicator->userId=name;

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
        sendMsg(filename+";"+filename+".apo;"+filename+".eswc"+":Download");
    }
    else
    {
        sendMsg(filename+":Download");
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
        sendMsg(filename.left(filename.size()-4)+":LoadANO");
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




