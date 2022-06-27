#ifndef CHECKGLWIDGET_H
#define CHECKGLWIDGET_H
#include "CheckWidget.h"


class CheckGlWidget:public QWidget{
public:
    CheckGlWidget(QWidget* parent);
    ~CheckGlWidget();


    bool isglwidget(){return cw_glwidget==nullptr?false:true;}
private:
    void drawlayout();

    MainWindow *mparent;
    V3dR_GLWidget * cw_glwidget;
    XFormWidget * cw_xfwidget;

    QPushButton *level1;
    QPushButton *level2;
    QPushButton *level3;
    QPushButton *level4;

    QHBoxLayout *top;
    QHBoxLayout *bottom;

    QVBoxLayout *central;
};

#endif // CHECKGLWIDGET_H
