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
}
void ManageSocket::onreadyRead()
{
    if(dataInfo.dataReadedSize==0&&this->bytesAvailable()>=sizeof (qint32))
    {
        QDataStream in(this);
        in>>dataInfo.dataSize;dataInfo.dataReadedSize+=sizeof (qint32);
        if(dataInfo.dataSize<0) {
            qDebug()<<"error";return;
        }        if(this->bytesAvailable()>=dataInfo.dataSize-dataInfo.dataReadedSize)
        {
            QStringList list;
            while(dataInfo.dataSize!=dataInfo.dataReadedSize)
            {
                in>>dataInfo.stringOrFilenameSize>>dataInfo.filedataSize;
                QString messageOrFileName=QString::fromUtf8(this->read(dataInfo.stringOrFilenameSize),dataInfo.stringOrFilenameSize);
                if(dataInfo.filedataSize==0)
                {
                    list.push_back("00"+messageOrFileName);
                }else
                {
                    QByteArray block=this->read(dataInfo.dataSize-dataInfo.dataReadedSize);
                    QString filePath=QCoreApplication::applicationDirPath()+"/download/"+messageOrFileName;
                    if(!QDir(QCoreApplication::applicationDirPath()+"/download").exists())
                    {
                        QDir(QCoreApplication::applicationDirPath()).mkdir("download");
                    }
                    QFile file(filePath);
                    file.open(QIODevice::WriteOnly);
                    file.write(block);file.flush();
                    file.close();
                }
                dataInfo.dataReadedSize+=(2*sizeof (qint32)+dataInfo.stringOrFilenameSize+dataInfo.filedataSize);
            }
            resetDataInfo();
            processReaded(list);
        }
    }else
    {
        if(this->bytesAvailable()>=dataInfo.dataSize-dataInfo.dataReadedSize)
        {
            QDataStream in(this);
            QStringList list;
            while(dataInfo.dataSize!=dataInfo.dataReadedSize)
            {
                in>>dataInfo.stringOrFilenameSize>>dataInfo.filedataSize;
                QString messageOrFileName=QString::fromUtf8(this->read(dataInfo.stringOrFilenameSize),dataInfo.stringOrFilenameSize);
                if(dataInfo.filedataSize==0)
                {
                    list.push_back("00"+messageOrFileName);
                }else
                {
                    QByteArray block=this->read(dataInfo.dataSize-dataInfo.dataReadedSize);
                    QString filePath=QCoreApplication::applicationDirPath()+"/download/"+messageOrFileName;
                    if(!QDir(QCoreApplication::applicationDirPath()+"/download").exists())
                    {
                        QDir(QCoreApplication::applicationDirPath()).mkdir("download");
                    }
                    QFile file(filePath);
                    file.open(QIODevice::WriteOnly);
                    file.write(block);
                    file.close();
                }
                dataInfo.dataReadedSize+=(2*sizeof (qint32)+dataInfo.stringOrFilenameSize+dataInfo.filedataSize);
            }
            resetDataInfo();
            processReaded(list);
        }
    }
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
    this->waitForBytesWritten();
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
        QListWidget *listwidget=new QListWidget;
        listwidget->setWindowTitle("choose annotation file");
        if(type=="down")
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,SLOT(download(itemDoubleClicked(QListWidgetItem*))));
        else if(type=="load")
            connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                    this,SLOT(load(QListWidgetItem*)));
        listwidget->clear();
        for(int i=0;i<response.size()-2;i++)
        {
            listwidget->addItem(response.at(i));
        }
        listwidget->show();
    }else if(CommunPort.indexIn(msg)!=-1)
    {
        int port=CommunPort.cap(1).toInt();
        pmain->Communicator = new V3dR_Communicator;
        connect(pmain->Communicator,SIGNAL(load(QString)),pmain,SLOT(ColLoadANO(QString)));
        terafly::CViewer *cur_win = terafly::CViewer::getCurrent();

        pmain->Communicator->userName=name;

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addSeg(QString)),
                cur_win->getGLWidget(),SLOT(CollaAddSeg(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delSeg(QString)),
                cur_win->getGLWidget(),SLOT(CollaDelSeg(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addMarker(QString)),
                cur_win->getGLWidget(),SLOT(CollaAddMarker(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delMarker(QString)),
                cur_win->getGLWidget(),SLOT(CollaDelMarker(QString)));

        connect(cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(retypeSeg(QString)),
                cur_win->getGLWidget(),SLOT(CollretypeSeg(QString)));

        connect(this,SIGNAL(disconnected()),cur_win->getGLWidget()->TeraflyCommunicator,SLOT(deleteLater()));
        pmain->Communicator->socket->connectToHost(ip,port);
        if(!pmain->Communicator->socket->waitForConnected())
        {
            QMessageBox::information(0,tr("Manage "),
                             tr("connect failed"),
                             QMessageBox::Ok);
            return;
        }else if(pmain->Communicator->socket->state()==QAbstractSocket::ConnectedState)
        {
            QMessageBox::information(0,tr("Manage "),
                             tr("Connect sucess!"),
                             QMessageBox::Ok);
        }
        cur_win->getGLWidget()->TeraflyCommunicator=pmain->Communicator;
        int maxresindex = terafly::CImport::instance()->getResolutions()-1;
        IconImageManager::VirtualVolume* vol = terafly::CImport::instance()->getVolume(maxresindex);
        pmain->Communicator->ImageMaxRes = XYZ(vol->getDIM_H(),vol->getDIM_V(),vol->getDIM_D());
        pmain->teraflyVRView->setDisabled(false);
        pmain->collaborationVRView->setEnabled(true);
        pmain->collautotrace->setEnabled(false);
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
}

void ManageSocket::load(QListWidgetItem* item)
{
    QString filename=item->text().trimmed();
    if(filename.endsWith(".ano"))
        sendMsg(filename.left(filename.size()-4)+":LoadANO");
    else
        qDebug()<<"choose file with.ano";
}


