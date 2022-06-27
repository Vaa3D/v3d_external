#include "CheckGlWidget.h"

CheckGlWidget::CheckGlWidget(QWidget* parent)
{
    mparent=(MainWindow *)parent;
    cw_glwidget=nullptr;
    cw_xfwidget=nullptr;

    drawlayout();
}

CheckGlWidget::~CheckGlWidget()
{

}
#include <QScrollArea>
void CheckGlWidget::drawlayout()
{
    level1=new QPushButton("w"); //worse
    level1->setMinimumSize(70,50);
    level2=new QPushButton("b"); //bad
    level2->setMinimumSize(70,50);
    level3=new QPushButton("g"); //good
    level3->setMinimumSize(70,50);
    level4=new QPushButton("e"); //excellent
    level4->setMinimumSize(70,50);

    top=new QHBoxLayout;
    bottom=new QHBoxLayout;
    central=new QVBoxLayout;

    QScrollArea *glWidgetArea = new QScrollArea;
    glWidgetArea->setMinimumSize(300,300);
    if(cw_glwidget)  	glWidgetArea->setWidget((QWidget *)cw_glwidget);

    top->addWidget(glWidgetArea);


    bottom->addWidget(level1);
    bottom->addWidget(level2);
    bottom->addWidget(level3);
    bottom->addWidget(level4);

    central->addLayout(top);
    central->addLayout(bottom);
    this->setLayout(central);
}
