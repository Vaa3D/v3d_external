#ifndef CSMAINWINDOW_H
#define CSMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class CSMainWindow;
}

class CSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CSMainWindow(QWidget *parent = nullptr);
    ~CSMainWindow();

private:
    Ui::CSMainWindow *ui;
};

#endif // CSMAINWINDOW_H
