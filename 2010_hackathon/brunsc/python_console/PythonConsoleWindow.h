#ifndef V3D_PYTHON_CONSOLE_WINDOW_H
#define V3D_PYTHON_CONSOLE_WINDOW_H

#include <QMainWindow>
#include "ui_main_python_console.h"
#include "PythonInterpreter.h"
#include "PythonOutputRedirector.h"

class QWidget;

namespace v3d {

class PythonConsoleWindow : public QMainWindow, Ui::main_python_console
{
	Q_OBJECT

public:
	PythonConsoleWindow(QWidget *parent=NULL);
	bool eventFilter ( QObject * watched, QEvent * event );
	PythonInterpreter pythonInterpreter;

signals:
	void returnPressed();

private slots:
	void onReturnPressed();
	
private:
	QString getCurrentCommand();

	QString prompt;
	int promptLength;
	PythonOutputRedirector stderrRedirector;
	PythonOutputRedirector stdoutRedirector;
};

} // namespace v3d

#endif // V3D_PYTHON_CONSOLE_WINDOW_H
