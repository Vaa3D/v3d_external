/*
 * V3DPythonModule.cpp
 *
 *  Created on: Dec 22, 2010
 *      Author: Christopher M. Bruns
 */

#include "V3DPythonModule.h"
#include <boost/python.hpp>
#include "v3d_interface.h"
#include <exception>
#include <iostream>
#include "convert_qstring.h"

// Store a permanent pointer to the callback the V3DConsolePlugin was launched with.
V3DPluginCallback2 *v3d_callbackPtr;

#include "generated_code/v3d.main.cpp"

namespace v3d {

void initV3DPythonModule(V3DPluginCallback2 *callback)
{
    // automatically convert qstring
    // WARNING - might interfere with PyQt/PySide
    register_qstring_conversion();

    // load module of automatically generated wrappers
    initv3d();

    if (callback) v3d_callbackPtr = callback;
}

} // namespace v3d
