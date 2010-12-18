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

void test_python()
{
	// from web
  try {
    Py_Initialize();

    bp::object main_module((
      bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));

    bp::object main_namespace = main_module.attr("__dict__");

    bp::handle<> ignored(( PyRun_String( "print \"Hello, World\"",
                                     Py_file_input,
                                     main_namespace.ptr(),
                                     main_namespace.ptr() ) ));
  } catch( bp::error_already_set ) {
    PyErr_Print();
  }
}

int main(int argc, char *argv[])
{
	// test_python();
	// PythonInterpreter python;
	// python.runString("print 'Hello, World'");

    QApplication app(argc, argv);

    PythonConsoleWindow* pythonConsole = new PythonConsoleWindow();
    pythonConsole->interpretString("print 'Hello, World'");

    pythonConsole->show();

    return app.exec();
}
