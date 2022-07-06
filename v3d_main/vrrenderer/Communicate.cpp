#include "Communicate.h"
VRViewerl* VRViewerl::uniqueInstancel=0;
VRViewerr* VRViewerr::uniqueInstancer=0;


void VRViewerl::run()
{
    qDebug()<<"VRViewerl "<<QThread::currentThreadId();
    while(1){

        if(m_sharedmem.isAttached()){
            if(!m_sharedmem.detach()){
                qDebug()<<"Detach SharedMemory Failed!";
                continue;
            }
        }
        break;
    }

    while(!m_sharedmem.create(307200)) {
        qDebug()<<"create sharedmemory failed!";
    }

    while(1){
        QImage* leftQImage = new QImage(leftdata,m_nRenderWidth,m_nRenderHeight,QImage::Format_RGB888);
        leftQImage->mirror();
        *leftQImage=leftQImage->scaled(1920/2,1080);
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out<<(*leftQImage);
        //qDebug()<<buffer.size();
        m_sharedmem.lock();          //把数据输入时锁定该共享内存段，其他进程将不能访问该共享内存
        //qDebug()<<m_sharedmem.data();
        char *to = (char*)m_sharedmem.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(int(buffer.size()), 307200));      //使用memcpy把要写入的数据拷贝入共享内存
        m_sharedmem.unlock();

    }
}

void VRViewerr::run()
{
    qDebug()<<"VRViewerr "<<QThread::currentThreadId();
    while(1){

        if(m_sharedmem.isAttached()){
            if(!m_sharedmem.detach()){
                qDebug()<<"Detach SharedMemory Failed!";
                continue;
            }
        }
        break;
    }

    while(!m_sharedmem.create(307200)) {
        qDebug()<<"create sharedmemory failed!";
    }

    while(1){
        QImage* rightQImage = new QImage(rightdata,m_nRenderWidth,m_nRenderHeight,QImage::Format_RGB888);
        rightQImage->mirror();
        *rightQImage=rightQImage->scaled(1920/2,1080);
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out<<(*rightQImage);
        //qDebug()<<buffer.size();
        m_sharedmem.lock();          //把数据输入时锁定该共享内存段，其他进程将不能访问该共享内存
        //qDebug()<<m_sharedmem.data();
        char *to = (char*)m_sharedmem.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(int(buffer.size()), 307200));      //使用memcpy把要写入的数据拷贝入共享内存
        m_sharedmem.unlock();

    }
}
