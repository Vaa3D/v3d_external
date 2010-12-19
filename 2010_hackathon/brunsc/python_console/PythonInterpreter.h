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

namespace v3d {

class PythonInterpreter {
public:
	PythonInterpreter();
	virtual ~PythonInterpreter();
	std::string interpretString(const std::string& cmd);

	boost::python::object main_module;
	boost::python::object main_namespace;
};

} // namespace v3d

#endif // V3D_PYTHON_INTERPRETER_H
