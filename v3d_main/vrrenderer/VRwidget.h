#ifndef VRWIDGET_H
#define VRWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLayout>

class VRwidget:public QWidget
{
    Q_OBJECT
public:
    VRwidget();
    ~VRwidget();
    unsigned char* leftdata;
    unsigned char* rightdata;
    QLabel *leftlabel;
    QLabel *rightlabel;
    QHBoxLayout *layout;
    QPixmap leftmp;
    QPixmap rightmp;
    bool isvrclosed=false;
    void closeEvent(QCloseEvent *event);
};

#endif // VRWIDGET_H
