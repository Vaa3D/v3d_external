#ifndef CSMAINWINDOW_H
#define CSMAINWINDOW_H

#include <QMainWindow>
#include "serverconnection/checkmapwidget.h"

namespace Ui {
class CSMainWindow;
}

class CSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CSMainWindow(QWidget *parent = nullptr);
    ~CSMainWindow();


public:
    CheckMapWidget *checkMapWidget;

private slots:
    void on_checkButton_clicked();

private:
    Ui::CSMainWindow *ui;
};

#endif // CSMAINWINDOW_H
