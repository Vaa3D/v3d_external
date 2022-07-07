#include "CheckGlWidget.h"

CheckGlWidget::CheckGlWidget(QWidget* parent,int id)
{
    mparent=(MainWindow *)parent;
    cw_glwidget=nullptr;
    cw_xfwidget=nullptr;
    cw_window=nullptr;
    m_id=id;
    status=-1;
    path="C:\\Users\\admin\\Desktop\\21_seg_color.tif";
    //path="D:/A_DLcsz/DLtrain/fixed_data/result/2.tiff";
    drawlayout();
    connect(level1,SIGNAL(clicked()),this,SLOT(level1pushed()));
    connect(level2,SIGNAL(clicked()),this,SLOT(level2pushed()));
    connect(level3,SIGNAL(clicked()),this,SLOT(level3pushed()));
    connect(level4,SIGNAL(clicked()),this,SLOT(level4pushed()));
//    openimage();
}

CheckGlWidget::~CheckGlWidget()
{

}

void CheckGlWidget::openimage()
{
    cw_xfwidget = mparent->newImageWindow(path);
    Image4DSimple *img=new Image4DSimple;
    img->loadImage(path.toLatin1().data());
    mparent->setImage((void *)cw_xfwidget,img);
    cw_xfwidget->hide();

    cw_xfwidget->doImage3DView(true, 0, -1, -1,-1, -1, -1,-1, false);
    cw_glwidget=cw_xfwidget->getView3D();
    cw_window=cw_glwidget->getiDrawExternalParameter()->window3D;
//    cw_window->hideDisplayControls();
//    cw_glwidget->resize(300,300);
    glWidgetArea->setWidget(cw_glwidget);
    //cw_glwidget->setMinimumSize(300,300);
    //cw_glwidget->move(0,0);
//    cw_glwidget->show();
//    this->show();


}

void CheckGlWidget::level1pushed()
{
    if(!this->cw_glwidget){
        level1->setChecked(false);
        level2->setChecked(false);
        level3->setChecked(false);
        level4->setChecked(false);
        status=-1;
        return;
    }
    switch(status){
    case -1:{
        level1->setChecked(true);
        status=1;
        break;
    }
    case 1:{
        level1->setChecked(false);
        status=-1;
        break;
    }
    case 2:{
        level2->setChecked(false);
        level1->setChecked(true);
        status=1;
        break;
    }
    case 3:{
        level3->setChecked(false);
        level1->setChecked(true);
        status=1;
        break;
    }
    case 4:{
        level4->setChecked(false);
        level1->setChecked(true);
        status=1;
        break;
    }
    }
}

void CheckGlWidget::level2pushed()
{
    if(!this->cw_glwidget){
        level1->setChecked(false);
        level2->setChecked(false);
        level3->setChecked(false);
        level4->setChecked(false);
        status=-1;
        return;
    }
    switch(status){
    case -1:{
        level2->setChecked(true);
        status=2;
        break;
    }
    case 1:{
        level1->setChecked(false);
        level2->setChecked(true);
        status=2;
        break;
    }
    case 2:{
        level2->setChecked(false);
        status=-1;
        break;
    }
    case 3:{
        level3->setChecked(false);
        level2->setChecked(true);
        status=2;
        break;
    }
    case 4:{
        level4->setChecked(false);
        level2->setChecked(true);
        status=2;
        break;
    }
    }
}

void CheckGlWidget::level3pushed()
{
    if(!this->cw_glwidget){
        level1->setChecked(false);
        level2->setChecked(false);
        level3->setChecked(false);
        level4->setChecked(false);
        status=-1;
        return;
    }
    switch(status){
    case -1:{
        status=3;
        level3->setChecked(true);
        break;
    }
    case 1:{
        level1->setChecked(false);
        level3->setChecked(true);
        status=3;
        break;
    }
    case 2:{
        level2->setChecked(false);
        level3->setChecked(true);
        status=3;
        break;
    }
    case 3:{
        level3->setChecked(false);
        status=-1;
        break;
    }
    case 4:{
        level4->setChecked(false);
        level3->setChecked(true);
        status=3;
        break;
    }
    }
}

void CheckGlWidget::level4pushed()
{
    if(!this->cw_glwidget){
        level1->setChecked(false);
        level2->setChecked(false);
        level3->setChecked(false);
        level4->setChecked(false);
        status=-1;
        return;
    }
    switch(status){
    case -1:{
        status=4;
        level4->setChecked(true);
        break;
    }
    case 1:{
        level1->setChecked(false);
        level4->setChecked(true);
        status=4;
        break;
    }
    case 2:{
        level2->setChecked(false);
        level4->setChecked(true);
        status=4;
        break;
    }
    case 3:{
        level3->setChecked(false);
        level4->setChecked(true);
        status=4;
        break;
    }
    case 4:{
        level4->setChecked(false);
        status=-1;
        break;
    }
    }
}
#include <QScrollArea>
void CheckGlWidget::drawlayout()
{
    level1=new QPushButton("w"); //worse
    level1->setMinimumSize(70,50);
    level1->setCheckable(true);
    level2=new QPushButton("b"); //bad
    level2->setMinimumSize(70,50);
    level2->setCheckable(true);
    level3=new QPushButton("g"); //good
    level3->setMinimumSize(70,50);
    level3->setCheckable(true);
    level4=new QPushButton("e"); //excellent
    level4->setMinimumSize(70,50);
    level4->setCheckable(true);

    top=new QHBoxLayout;
    bottom=new QHBoxLayout;
    central=new QVBoxLayout;

    glWidgetArea = new QScrollArea;
    glWidgetArea->setMinimumSize(300,300);
    if(cw_glwidget)  	glWidgetArea->setWidget(cw_glwidget);
    m_id_label=new QLabel(QString::number(m_id));
    m_id_label->setAlignment(Qt::AlignHCenter);
    top->addWidget(m_id_label);
    top->addWidget(glWidgetArea);

    bottom->addWidget(level1);
    bottom->addWidget(level2);
    bottom->addWidget(level3);
    bottom->addWidget(level4);

    central->addLayout(top);
    central->addLayout(bottom);
    this->setLayout(central);
}
