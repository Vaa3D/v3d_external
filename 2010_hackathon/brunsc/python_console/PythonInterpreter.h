#ifndef V3D_PYTHON_INTERPRETER_H
#define V3D_PYTHON_INTERPRETER_H

/*
 * PythonInterpreter.h
 *
 *  Created on: Dec 18, 2010
 *      Author: Christopher M. Bruns
 */

#include <string>

namespace v3d {

class PythonInterpreter {
public:
	PythonInterpreter();
	virtual ~PythonInterpreter();
	void interpretString(const std::string& cmd);

private:
};

} // namespace v3d

#endif // V3D_PYTHON_INTERPRETER_H
