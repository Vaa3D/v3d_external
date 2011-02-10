#include "PythonEditorWidget.h"
#include <iostream>

using namespace std;

PythonEditorWidget::PythonEditorWidget(QWidget* parent)
        : QMainWindow(parent)
{
    cerr << "constructor" << endl;   
    setupUi(this);
    setWindowIcon(QIcon(":/icons/python.png"));
}

