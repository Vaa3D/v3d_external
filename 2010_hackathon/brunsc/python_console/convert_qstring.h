/*
 * convert_qstring.h
 *
 *  Created on: Dec 23, 2010
 *      Author: cmbruns
 *
 *  Interconvert between QString and python string.
 *  Warning: use of this technique might be incompatible with
 *  PyQt or PySide
 */

// Call this from within BOOST_PYTHON_MODULE block
void register_qstring_conversion();
