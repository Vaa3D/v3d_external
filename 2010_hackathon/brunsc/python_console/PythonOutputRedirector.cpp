/*
 * PythonOutputRedirector.cpp
 *
 *  Created on: Dec 19, 2010
 *      Author: Christopher M. Bruns
 */

#include "PythonOutputRedirector.h"
#include <QPlainTextEdit>
#include <QString>

void PythonOutputRedirector::setQPlainTextEdit(QPlainTextEdit* p_textEdit)
{
    textEdit = p_textEdit;
}

void PythonOutputRedirector::write( std::string const& str )
{
	// Append to console
	// textEdit->appendPlainText( str.c_str() ); // no good, adds newlines
	textEdit->moveCursor(QTextCursor::End);
	textEdit->insertPlainText( str.c_str() );
}
