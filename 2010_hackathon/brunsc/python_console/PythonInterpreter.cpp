#include "PythonInterpreter.h"

namespace bp = boost::python;
using namespace std;

namespace v3d {

PythonInterpreter::PythonInterpreter()
{
	try {
		Py_Initialize();
		main_module = bp::object((
		  bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
		main_namespace = main_module.attr("__dict__");
	}
	catch( bp::error_already_set ) {
		PyErr_Print();
	}
}

PythonInterpreter::~PythonInterpreter() {
	Py_Finalize();
}

std::string PythonInterpreter::interpretString(const std::string& cmd)
{
	try {
		bp::object result((bp::handle<>( PyRun_String(
				cmd.c_str(),
				Py_single_input,
				main_namespace.ptr(),
				main_namespace.ptr() ) )));
		const char* resultString = bp::extract<const char*>(result);
		if (resultString)
			return resultString;
	}
	catch( bp::error_already_set ) {
		PyErr_Print();
	}

	return "";
}

} // namespace v3d
