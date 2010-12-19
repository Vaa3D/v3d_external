/*
 * pythonConsoleTest.cpp
 *
 *  Created on: Dec 18, 2010
 *      Author: Christopher M. Bruns
 */

#include "PythonInterpreter.h"
#include "PythonConsoleWindow.h"
#include <boost/python.hpp>

using namespace v3d;
namespace bp = boost::python;

int main(int argc, char *argv[])
{
	// test_python();
	// PythonInterpreter python;
	// python.runString("print 'Hello, World'");

    QApplication app(argc, argv);

    PythonConsoleWindow* pythonConsole = new PythonConsoleWindow();
    // pythonConsole->pythonInterpreter.interpretString("print 'Hello, World'");

    pythonConsole->show();

    return app.exec();
}
