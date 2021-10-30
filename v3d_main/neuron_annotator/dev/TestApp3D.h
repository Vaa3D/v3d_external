/*
 * TestApp3D.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef TESTAPP3D_H_
#define TESTAPP3D_H_

#include <QMainWindow>
#include "ui_TestApp3D.h"

class TestApp3D : public QMainWindow
{
public:
    TestApp3D();
    virtual ~TestApp3D();

protected:
    Ui::MainWindow ui;
};

#endif /* TESTAPP3D_H_ */
