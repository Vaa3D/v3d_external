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
#include <QObject>

class PythonInterpreter;

class PythonOutputRedirector2
{
public:
	PythonOutputRedirector2(PythonInterpreter *interpreter = NULL);
    void write( std::string const& str );

private:
	PythonInterpreter *interpreter;
};

class PythonInputRedirector
{
public:
	PythonInputRedirector(PythonInterpreter *interpreter = NULL);
    std::string readline( );

private:
    PythonInterpreter *interpreter;
};

class PythonInterpreter : public QObject
{
	Q_OBJECT

public:
	// class IncompletePythonCommandException : public std::exception {};

	PythonInterpreter();
	virtual ~PythonInterpreter();
	// std::string interpretString(const std::string& cmd)
	// 	throw(IncompletePythonCommandException);

signals:
	void outputSent(QString msg);
	void readLine();
	void commandComplete();
	void incompleteCommand(QString partialCmd);

public slots:
	void interpretLine(QString line);
	void onOutput(QString msg);

private:
	boost::python::object main_module;
	boost::python::object main_namespace;
    PythonInputRedirector stdinRedirector;
	PythonOutputRedirector2 stdoutRedirector;
    PythonOutputRedirector2 stderrRedirector;
};

#endif // V3D_PYTHON_INTERPRETER_H
