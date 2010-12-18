/*
 * pythonConsole.cpp
 *
 *  Created on: Dec 18, 2010
 *      Author: cmbruns
 */

#include "PythonConsoleWindow.h"
#include <iostream>

namespace v3d {

PythonConsoleWindow::PythonConsoleWindow(QWidget *parent)
		: QMainWindow(parent), prompt("")
{
	setupUi(this);

	promptLength = prompt.length();

	// Run python command after user presses return
	plainTextEdit->installEventFilter(this);
	connect(this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
}

void PythonConsoleWindow::interpretString(const QString& cmd) {
	pythonInterpreter.interpretString(cmd.toStdString());
}

// When user presses RETURN key in text area, execute the python command
bool PythonConsoleWindow::eventFilter ( QObject * watched, QEvent * event )
{
	if (watched != plainTextEdit) // not the text area
		return QMainWindow::eventFilter(watched, event);

	if (event->type() != QEvent::KeyPress) // not a keyboard event
		return QMainWindow::eventFilter(watched, event);

	QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
	switch(keyEvent->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit returnPressed();
			break;
	}

	return QMainWindow::eventFilter(watched, event);
}

void PythonConsoleWindow::onReturnPressed()
{
	QString command = getCurrentCommand();
	std::cerr << "Return..." << std::endl;
	std::cerr << "Command = '" << command.toStdString() << "'" << std::endl;
	interpretString(command);
}

QString PythonConsoleWindow::getCurrentCommand()
{
	QTextCursor cursor = plainTextEdit->textCursor();
	cursor.movePosition(QTextCursor::StartOfLine);
	// Skip over the prompt, and begin the selection
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLength);
	// End selection at end of line
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	QString command = cursor.selectedText();
	// TODO - restore any previous selection
	cursor.clearSelection();
	return command;
}

} // namespace v3d
