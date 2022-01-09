#include "VRwidget.h"


VRwidget::VRwidget()
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
}

VRwidget::~VRwidget()
{
    if(isvrclosed==true)
        qDebug()<<"the vrwidget is closed.";
}

#include <QCloseEvent>
void VRwidget::closeEvent(QCloseEvent *event)
{
    qDebug()<<"Close the VR through VRviewer!";
    isvrclosed=true;
    event->accept();
}
