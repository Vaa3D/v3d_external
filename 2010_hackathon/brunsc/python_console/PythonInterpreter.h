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

class PythonOutputRedirector2 : public QObject
{
	Q_OBJECT

public:
	PythonOutputRedirector2(QObject* parent = NULL) : QObject(parent) {}
    void write( std::string const& str );

signals:
	void output(QString msg);
};

class PythonInputRedirector : public QObject
{
	Q_OBJECT

public:
	PythonInputRedirector(QObject* parent = NULL) : QObject(parent) {}
    std::string readline( );
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
	void stdErr(QString msg);
	void stdOut(QString msg);
	void readLine();
	void commandComplete();
	void incompleteCommand(QString partialCmd);

public slots:
	void interpretLine(QString line);

private:
	boost::python::object main_module;
	boost::python::object main_namespace;
    PythonInputRedirector stdinRedirector;
	PythonOutputRedirector2 stdoutRedirector;
    PythonOutputRedirector2 stderrRedirector;
};

#endif // V3D_PYTHON_INTERPRETER_H
