#include "PythonInterpreter.h"
#include <iostream>
#include <cstdio>

namespace bp = boost::python;
using namespace std;

PythonOutputRedirector::PythonOutputRedirector(PythonInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

void PythonOutputRedirector::write( std::string const& str )
{
    interpreter->onOutput(QString(str.c_str()));
}

PythonInputRedirector::PythonInputRedirector(PythonInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

// TODO - this is a hack that does not actually get user input
std::string PythonInputRedirector::readline()
{
    return interpreter->readline();
    // cerr << "readline" << endl;
    // return "\n"; // TODO this is a hack to avoid hanging during input.
}

std::string PythonInterpreter::readline()
{
    emit startReadline();
    readlineLoop.exec(); // block until readline
    return readlineString.toStdString();
}

PythonInterpreter::PythonInterpreter()
	: QObject(NULL),
	  stdinRedirector(this),
	  stdoutRedirector(this),
	  stderrRedirector(this)
{
	try {
		Py_Initialize();
		main_module = bp::object((
		  bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
		main_namespace = main_module.attr("__dict__");

		// Connect python stdout/stderr to output to GUI
		// Adapted from
		//   http://onegazhang.spaces.live.com/blog/cns!D5E642BC862BA286!727.entry
		main_namespace["PythonOutputRedirector"] =
			bp::class_<PythonOutputRedirector>(
					"PythonOutputRedirector", bp::init<>())
				.def("write", &PythonOutputRedirector::write)
				;
		main_namespace["PythonInputRedirector"] =
			bp::class_<PythonInputRedirector>(
					"PythonInputRedirector", bp::init<>())
				.def("readline", &PythonInputRedirector::readline)
				;

	    bp::import("sys").attr("stdin") = stdinRedirector;
	    bp::import("sys").attr("stdout") = stdoutRedirector;
		bp::import("sys").attr("stderr") = stderrRedirector;
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

void PythonInterpreter::onOutput(QString msg) {
    emit outputSent(msg);
}

void PythonInterpreter::runScriptFile(QString fileName)
{
	FILE *fp = fopen(fileName.toLocal8Bit(), "r");
	if (fp) {
		PyRun_SimpleFileEx(fp, fileName.toLocal8Bit(), 1); // 1 means close it for me
	}
	emit commandComplete();
}

void PythonInterpreter::finishReadline(QString line)
{
    // Don't run command if python stdin is waiting for readline input
    readlineString = line;
    readlineLoop.exit();
    return;
}

void PythonInterpreter::interpretLine(QString line)
// std::string PythonInterpreter::interpretString(const std::string& command0)
// 	throw(IncompletePythonCommandException)
{
	std::string command0 = line.toStdString();

	// Skip empty lines
	if (command0.length() == 0) {
		emit commandComplete();
		return; // empty command
	}
	size_t firstNonSpacePos = command0.find_first_not_of(" \t\r\n");
	if (firstNonSpacePos == std::string::npos) {
		emit commandComplete();
		return; // all blanks command
	}
	if (command0[firstNonSpacePos] == '#') {
		emit commandComplete();
		return; // comment line
	}
	// Append newline for best parsing of nascent multiline commands.
	std::string command = command0 + "\n";

	try {
		// First compile the expression without running it.
		bp::object compiledCode(bp::handle<>(Py_CompileString(
				command.c_str(),
				"<stdin>",
				Py_single_input)));
		if (! compiledCode.ptr()) {
			// command failed
			emit commandComplete();
			return;
		}

		bp::object result(bp::handle<>( PyEval_EvalCode(
				(PyCodeObject*) compiledCode.ptr(),
				main_namespace.ptr(),
				main_namespace.ptr())));
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
				emit incompleteCommand(line);
				return;
			}
			PyErr_Restore (exc, val, trb);
		}

		PyErr_Print();
	}

	emit commandComplete();
	return;
}
