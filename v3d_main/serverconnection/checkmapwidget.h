#ifndef CHECKMAPWIDGET_H
#define CHECKMAPWIDGET_H

#include <QWidget>

#include "net/httputilsimage.h"

namespace Ui {
class CheckMapWidget;
}

class CheckMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CheckMapWidget(QWidget *parent = nullptr);
    ~CheckMapWidget();

//    void initHttp();
    // 1. 从服务器获取到数据
//    void downloadImag();

    // 2. 渲染数据并显示

private:
    Ui::CheckMapWidget *ui;

    HttpUtilsImage *imageUtils;
};

#endif // CHECKMAPWIDGET_H
