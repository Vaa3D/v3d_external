#ifndef COMMUNICATE_H
#define COMMUNICATE_H

#include <QThread>
#include <QBuffer>
#include <QSharedMemory>
#include <QDebug>
#include <QImage>

class VRViewerl:public QThread{
    Q_OBJECT
private:
    static VRViewerl* uniqueInstancel;
    VRViewerl():QThread(){
        m_sharedmem.setKey("v3dcszfortestl");
        flag=0;
    }

    void run();

public:
    static VRViewerl* instance(){
        if(!uniqueInstancel)
            uniqueInstancel=new VRViewerl();
        return uniqueInstancel;
    }
    unsigned char *leftdata;
    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;
    QSharedMemory m_sharedmem;
    int flag;
};

class VRViewerr:public QThread{
    Q_OBJECT
private:
    static VRViewerr* uniqueInstancer;
    VRViewerr():QThread(){
        m_sharedmem.setKey("v3dcszfortestr");
        flag=0;
    }

    void run();

public:
    static VRViewerr* instance(){
        if(!uniqueInstancer)
            uniqueInstancer=new VRViewerr();
        return uniqueInstancer;
    }
    unsigned char *rightdata;
    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;
    QSharedMemory m_sharedmem;
    int flag;
};

#endif // COMMUNICATE_H
