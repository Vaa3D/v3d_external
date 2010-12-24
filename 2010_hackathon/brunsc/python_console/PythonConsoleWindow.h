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
	PythonConsoleWindow(QWidget *parent = NULL);
	void executeCommand(const QString& command);
    bool eventFilter(QObject *watched, QEvent *event);

signals:
	void returnPressed();
	void pasteAvailable(bool);
	void cutAvailable(bool);

private slots:
	void onReturnPressed();
	void onCursorPositionChanged();
	void onClipboardDataChanged();
	void onSelectionChanged();
	void onCopyAvailable(bool);
	void about();
	
private:
	void setupMenus();
	QString getCurrentCommand();
	void placeNewPrompt(bool bMakeVisible=false);
	void setPrompt(const QString& newPrompt);
	bool cursorIsInEditingRegion(const QTextCursor& cursor);

	QString prompt;
	int promptLength;
	PythonOutputRedirector stderrRedirector;
	PythonOutputRedirector stdoutRedirector;
	QTextCursor latestGoodCursorPosition;
	QString multilineCommand;
    PythonInterpreter pythonInterpreter;
};

#endif // V3D_PYTHON_CONSOLE_WINDOW_H
