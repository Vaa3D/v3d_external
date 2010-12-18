#ifndef V3D_PYTHON_CONSOLE_WINDOW_H
#define V3D_PYTHON_CONSOLE_WINDOW_H

#include <QMainWindow>
#include "ui_main_python_console.h"
#include "PythonInterpreter.h"

class QWidget;

namespace v3d {

class PythonConsoleWindow : public QMainWindow, Ui::main_python_console
{
	Q_OBJECT

public:
	PythonConsoleWindow(QWidget *parent=NULL);
	void interpretString(const QString& cmd);
	bool eventFilter ( QObject * watched, QEvent * event );

signals:
	void returnPressed();

private slots:
	void onReturnPressed();
	
private:
	QString getCurrentCommand();

	PythonInterpreter pythonInterpreter;
	int promptLength;
	QString prompt;
};

} // namespace v3d

#endif // V3D_PYTHON_CONSOLE_WINDOW_H
