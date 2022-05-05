#include "checkmapwidget.h"
#include "ui_checkmapwidget.h"

/**
 * @brief 此窗口在构造时完成图像下载及渲染
 * @param parent
 */
CheckMapWidget::CheckMapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CheckMapWidget)
{
    ui->setupUi(this);

//    downloadImage();
//    renderImage();

}

CheckMapWidget::~CheckMapWidget()
{
    delete ui;
}
