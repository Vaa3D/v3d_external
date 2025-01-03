#ifndef CHECKGLWIDGET_H
#define CHECKGLWIDGET_H
#include "CheckWidget.h"


class CheckManager;
class CheckGlWidget:public QWidget{
    Q_OBJECT
public:
    CheckGlWidget(QWidget* parent,int id);
    ~CheckGlWidget();


    bool isglwidget(){return cw_glwidget==nullptr?false:true;}
    int getstatus(){return status;};
    void clear();
    QString imgname;
public slots:
    void openimage(QString extern_path);
    void level1pushed();
    void level2pushed();
    void level3pushed();
    void level4pushed();
private:
    void drawlayout();

    MainWindow *mparent;
    V3dR_GLWidget * cw_glwidget;
    XFormWidget * cw_xfwidget;
    V3dR_MainWindow *cw_window;
    int m_id;
    QLabel *m_id_label;
    int status;
    QString path;
    QPushButton *level1;
    QPushButton *level2;
    QPushButton *level3;
    QPushButton *level4;
    QScrollArea *glWidgetArea;
    QHBoxLayout *top;
    QHBoxLayout *bottom;


    QScrollBar *sb;
    QVBoxLayout *central;
};

#endif // CHECKGLWIDGET_H
