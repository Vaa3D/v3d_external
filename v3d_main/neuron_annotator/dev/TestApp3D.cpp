/*
 * TestApp3D.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#include "TestApp3D.h"
#include <QApplication>

TestApp3D::TestApp3D()
{
    ui.setupUi(this);
}

TestApp3D::~TestApp3D()
{
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    TestApp3D mainWindow;
    mainWindow.show();

    return app.exec();
}
