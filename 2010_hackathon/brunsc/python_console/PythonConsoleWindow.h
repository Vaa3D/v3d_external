#ifndef V3D_PYTHON_CONSOLE_WINDOW_H
#define V3D_PYTHON_CONSOLE_WINDOW_H

#include <QMainWindow>
#include "ui_main_python_console.h"
#include "PythonInterpreter.h"
#include "PythonOutputRedirector.h"

class QWidget;

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
	void onCursorPositionChanged();
	
private:
	QString getCurrentCommand();
	void placeNewPrompt(bool bMakeVisible=false);
	void setPrompt(const QString& newPrompt);

	QString prompt;
	int promptLength;
	PythonOutputRedirector stderrRedirector;
	PythonOutputRedirector stdoutRedirector;
	QTextCursor latestGoodCursorPosition;
	QString multilineCommand;
};

#endif // V3D_PYTHON_CONSOLE_WINDOW_H
