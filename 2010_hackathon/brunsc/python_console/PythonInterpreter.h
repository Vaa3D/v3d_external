#ifndef V3D_PYTHON_INTERPRETER_H
#define V3D_PYTHON_INTERPRETER_H

/*
 * PythonInterpreter.h
 *
 *  Created on: Dec 18, 2010
 *      Author: Christopher M. Bruns
 */

#include <boost/python.hpp>
#include <string>
#include <exception>

class PythonInterpreter {
public:
	class IncompletePythonCommandException : public std::exception {};

	PythonInterpreter();
	virtual ~PythonInterpreter();
	std::string interpretString(const std::string& cmd)
		throw(IncompletePythonCommandException);

	boost::python::object main_module;
	boost::python::object main_namespace;
};

#endif // V3D_PYTHON_INTERPRETER_H
