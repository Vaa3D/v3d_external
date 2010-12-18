#include "PythonInterpreter.h"

#include <boost/python.hpp>

namespace bp = boost::python;
using namespace std;

namespace v3d {

PythonInterpreter::PythonInterpreter() {
	Py_Initialize();
}

PythonInterpreter::~PythonInterpreter() {
	Py_Finalize();
}

void PythonInterpreter::interpretString(const std::string& cmd) {
	PyRun_SimpleString(cmd.c_str());
}

} // namespace v3d
