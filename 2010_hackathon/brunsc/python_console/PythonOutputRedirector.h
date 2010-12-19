/*
 * PythonOutputRedirector.h
 *
 *  Created on: Dec 19, 2010
 *      Author: Christopher M. Bruns
 */

#ifndef V3D_PYTHONOUTPUTREDIRECTOR_H_
#define V3D_PYTHONOUTPUTREDIRECTOR_H_

#include <string>
class QPlainTextEdit;

namespace v3d {

// PythonOutputRedirector is intended to be wrapped with boost::python,
// and to replace sys.stderr or sys.stdout, to send the output to the
// V3D python console.
// Based on example at
// http://onegazhang.spaces.live.com/blog/cns!D5E642BC862BA286!727.entry
class PythonOutputRedirector
{
public:
	PythonOutputRedirector() {}
	void setQPlainTextEdit(QPlainTextEdit* textEdit);
    void write( std::string const& str );

private:
    QPlainTextEdit* textEdit;
};

} // namespace v3d

#endif /* V3D_PYTHONOUTPUTREDIRECTOR_H_ */
