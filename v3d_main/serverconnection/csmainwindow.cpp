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
