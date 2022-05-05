#include "csmainwindow.h"
#include "ui_csmainwindow.h"


CSMainWindow::CSMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CSMainWindow)
{
    ui->setupUi(this);
}

CSMainWindow::~CSMainWindow()
{
    delete ui;
}

void CSMainWindow::on_checkButton_clicked()
{
    checkMapWidget = new CheckMapWidget(this);
    checkMapWidget->show();
}

