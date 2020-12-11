#include "managesocket.h"
#include <QDataStream>
#include <QFile>
#include <QCoreApplication>
#include <QRegExp>
#include <QTime>
#include <QListWidget>
#include <QListWidgetItem>
#include "V3dR_Communicator.h"
#include "../../terafly/src/control/CImport.h"

ManageSocket::ManageSocket(QObject *parent):QTcpSocket(parent)
{
    {
        resetDataInfo();
        msgs.clear();
        filepaths.clear();
    }
    connect(this,SIGNAL(readyread()),this,SLOT(onreadyRead()));
}
void ManageSocket::onreadyRead()
{
    if(dataInfo.dataReadedSize==0&&this->bytesAvailable()>=sizeof (qint32))
    {
        QDataStream in(this);
        in>>dataInfo.dataSize;dataInfo.dataReadedSize+=sizeof (qint32);
        if(dataInfo.dataSize<0) {
            qDebug()<<"error";return;
        }
        if(dataInfo.dataSize==dataInfo.dataReadedSize)
        {
            resetDataInfo();return;
        }

        if(this->bytesAvailable()>=dataInfo.dataSize-dataInfo.dataReadedSize)
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
                    QString filePath=QCoreApplication::applicationDirPath()+"/data/"+messageOrFileName;
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
                    QString filePath=QCoreApplication::applicationDirPath()+"/data/"+messageOrFileName;
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
    qint32 totalsize=sizeof (qint32);
    QByteArray block1;
    QDataStream dts(&block1,QIODevice::WriteOnly);
    for(int i=0;i<filePathList.size();i++)
    {
        auto fileName=fileNameList[i].toUtf8();
        auto fileData=QFile(filePathList[i]).readAll();
        totalsize += 2* sizeof (qint32);
        totalsize += fileName.size();
        totalsize += fileData.size();
        dts<<qint32(fileName.size())<<qint32(fileData.size());
        block1 += fileName +=fileData;
    }
    QByteArray block;
    QDataStream dts1(&block,QIODevice::WriteOnly);
    dts<<qint32(totalsize);
    block+=block1;
    this->write(block);
    this->waitForBytesWritten();
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
            processFile(filepaths);
            msgs.push_back(msg.remove(0,2));
        }else if(msg.startsWith("11"))
        {
            processMsg(msgs);
            filepaths.push_back(msg.remove(0,2));
        }
    }
    processMsg(msgs);
    processFile(filepaths);
}
void ManageSocket::processMsg( QStringList &msgs)
{
     for(auto msg:msgs)
     {
        QRegExp FileList("(.*):CurrentFiles");//down;data:CurrentFiles
        QRegExp CommunPort("(.*):Port");
        if(FileList.indexIn(msg)!=-1)
        {
            QStringList response=FileList.cap(1).trimmed().split(";");
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
            int port=CommunPort.cap(1).toUInt();
            pmain->teraflyVRView->setDisabled(true);
            pmain->collaborationVRView->setEnabled(true);
            pmain->collautotrace->setEnabled(false);

            pmain->Communicator = new V3dR_Communicator;
            pmain->Communicator->userName=name;
            connect(pmain->Communicator,SIGNAL(load(QString)),this,SLOT(ColLoadANO(QString)));
            pmain->cur_win->getGLWidget()->TeraflyCommunicator=pmain->Communicator;

//            connect(this,SIGNAL(signal_communicator_read_res(QString,XYZ*)),
//                    cur_win->getGLWidget()->TeraflyCommunicator,SLOT(read_autotrace(QString,XYZ*)));//autotrace

            connect(pmain->cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addSeg(QString)),
                    pmain->cur_win->getGLWidget(),SLOT(CollaAddSeg(QString)));

            connect(pmain->cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delSeg(QString)),
                    pmain->cur_win->getGLWidget(),SLOT(CollaDelSeg(QString)));

            connect(pmain->cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(addMarker(QString)),
                    pmain->cur_win->getGLWidget(),SLOT(CollaAddMarker(QString)));

            connect(pmain->cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(delMarker(QString)),
                    pmain->cur_win->getGLWidget(),SLOT(CollaDelMarker(QString)));

            connect(pmain->cur_win->getGLWidget()->TeraflyCommunicator,SIGNAL(retypeSeg(QString)),
                    pmain->cur_win->getGLWidget(),SLOT(CollretypeSeg(QString)));

            connect(this,SIGNAL(disconnected()),pmain->getGLWidget()->TeraflyCommunicator,SLOT(deleteLater()));

            int maxresindex = CImport::instance()->getResolutions()-1;
            VirtualVolume* vol = CImport::instance()->getVolume(maxresindex);
            pmain->Communicator->ImageMaxRes = XYZ(vol->getDIM_H(),vol->getDIM_V(),vol->getDIM_D());
//            pmain->Communicator
        }
    }
     msgs.clear();
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
        sendMsg(filename.chopped(4)+":LoadANO");

}


