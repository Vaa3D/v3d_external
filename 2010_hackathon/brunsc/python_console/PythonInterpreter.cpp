#include "PythonInterpreter.h"
#include <iostream>

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
	// TODO - using Py_Finalize() may be unsafe with boost::python
	// http://www.boost.org/libs/python/todo.html#pyfinalize-safety
	// http://lists.boost.org/Archives/boost/2006/07/107149.php
	Py_Finalize();
}

std::string PythonInterpreter::interpretString(const std::string& command)
	throw(IncompletePythonCommandException)
{
	// Skip empty lines
	if (command.length() == 0) return ""; // empty command
	size_t firstNonSpacePos = command.find_first_not_of(" \t\r\n");
	if (firstNonSpacePos == std::string::npos) return ""; // all blanks command
	if (command[firstNonSpacePos] == '#') return ""; // comment line

	try {
		// First compile the expression without running it.
		bp::object compiledCode(bp::handle<>(Py_CompileString(
				command.c_str(),
				"<stdin>",
				Py_single_input)));
		if (! compiledCode.ptr()) return "";

		bp::object result(bp::handle<>( PyEval_EvalCode(
				(PyCodeObject*) compiledCode.ptr(),
				main_namespace.ptr(),
				main_namespace.ptr())));

		const char* resultString = bp::extract<const char*>(result);

		if (resultString)
			return resultString;
	}
	catch( bp::error_already_set )
	{
		// Distinguish incomplete input from invalid input
		char *msg = NULL;
		PyObject *exc, *val, *obj, *trb;
		if (PyErr_ExceptionMatches(PyExc_SyntaxError))
		{
			PyErr_Fetch (&exc, &val, &trb);        /* clears exception! */
			if (PyArg_ParseTuple (val, "sO", &msg, &obj) &&
					!strcmp (msg, "unexpected EOF while parsing")) /* E_EOF */
			{
				Py_XDECREF (exc);
				Py_XDECREF (val);
				Py_XDECREF (trb);
				throw IncompletePythonCommandException();
			}
			PyErr_Restore (exc, val, trb);
		}

		PyErr_Print();
	}

	return "";
}

} // namespace v3d
