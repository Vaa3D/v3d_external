#include "VRwidget.h"


VRwidget::VRwidget()
{
//    leftdata=0;
//    rightdata=0;
    drawlayout();

}

VRwidget::~VRwidget()
{
    if(isvrclosed==true)
        qDebug()<<"the vrwidget is closed.";
//    if(leftdata){
//        delete leftdata;
//        leftdata=0;
//    }
//    if(rightdata){
//        delete rightdata;
//        rightdata=0;
//    }
    //qDebug()<<"csz debug VRwidget is destoryed.";
}
#include <QCoreApplication>
void VRwidget::seteye(QImage *texture, int eye)
{
    //qDebug()<<texture<<" has been loaded, the eye is "<<eye;
    if(eye==1){
        leftmp=leftmp.fromImage(*texture);
        this->leftlabel->setPixmap(leftmp);
    }else if(eye==2){
        rightmp=rightmp.fromImage(*texture);
        this->rightlabel->setPixmap(rightmp);
    }
    //QCoreApplication::processEvents();
}

void VRwidget::drawlayout()
{
    this->setWindowTitle("VRviewer");
    this->setFixedSize(1920,1080);
    leftlabel=new QLabel(this);
    leftlabel->setGeometry(0,0,1920/2,1080/2);
    leftlabel->setFixedSize(1920/2,1080);
    rightlabel=new QLabel(this);
    rightlabel->setGeometry(1920/2,0,1920/2,1080/2);
    rightlabel->setFixedSize(1920/2,1080);
    layout=new QHBoxLayout();
    leftlabel->setText("left eye");
    rightlabel->setText("right eye");
    layout->addWidget(leftlabel);
    layout->addWidget(rightlabel);
    this->setLayout(layout);
    isvrclosed=false;
    isdrawlayout=true;
}

#include <QCloseEvent>
void VRwidget::closeEvent(QCloseEvent *event)
{
    qDebug()<<"Close the VR through VRviewer!";
    isvrclosed=true;
    event->accept();
}
