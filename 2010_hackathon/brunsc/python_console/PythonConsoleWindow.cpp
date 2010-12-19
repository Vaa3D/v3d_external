/*
 * pythonConsole.cpp
 *
 *  Created on: Dec 18, 2010
 *      Author: cmbruns
 */

#include "PythonConsoleWindow.h"
#include <iostream>
#include <QTextBlock>

namespace bp = boost::python;

namespace v3d {

PythonConsoleWindow::PythonConsoleWindow(QWidget *parent)
		: QMainWindow(parent),
		  prompt(">>> "),
		  promptLength(prompt.length())
{
	setupUi(this);

	// Run python command after user presses return
	plainTextEdit->installEventFilter(this);
	connect(this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));

	// Connect python stdout/stderr to output to GUI
	// Adapted from
	//   http://onegazhang.spaces.live.com/blog/cns!D5E642BC862BA286!727.entry
	stdoutRedirector.setQPlainTextEdit(plainTextEdit);
	stderrRedirector.setQPlainTextEdit(plainTextEdit);
	pythonInterpreter.main_namespace["PythonStdIoRedirect"] =
			bp::class_<PythonOutputRedirector>(
					"V3DOutputRedirector",  bp::init<>())
				.def("write", &PythonOutputRedirector::write);
	bp::import("sys").attr("stderr") = stderrRedirector;
	bp::import("sys").attr("stdout") = stdoutRedirector;

	// TODO - print intro text at top
	std::string pyVersion("Python ");
	pyVersion += Py_GetVersion();
	pyVersion += " on ";
	pyVersion += Py_GetPlatform();
	plainTextEdit->appendPlainText(pyVersion.c_str());

	plainTextEdit->appendPlainText(
			"Type \"help\", \"copyright\", \"credits\" or "
			"\"license()\" for more information.");
	plainTextEdit->appendPlainText("V3D python console.");

	plainTextEdit->appendPlainText(""); // need new line for prompt
	placeNewPrompt(true);
}

// When user presses <Return> key in text area, execute the python command
bool PythonConsoleWindow::eventFilter ( QObject * watched, QEvent * event )
{
	if (watched != plainTextEdit) // we only care about the TextEdit widget
		return QMainWindow::eventFilter(watched, event);

	if (event->type() != QEvent::KeyPress) // we only care about keyboard events
		return QMainWindow::eventFilter(watched, event);

	QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
	switch(keyEvent->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			emit returnPressed();
			return true; // We will take care of inserting the newline.
	}

	return QMainWindow::eventFilter(watched, event);
}

void PythonConsoleWindow::onReturnPressed()
{
	// TODO - scroll down after command, iff bottom is visible now.
	bool endIsVisible = plainTextEdit->document()->lastBlock().isVisible();

	QString command = getCurrentCommand();
	// Add carriage return, so output will appear on subsequent line.
	// (It would be too late if we waited for plainTextEdit
	//  to process the <Return>)
	plainTextEdit->appendPlainText("");
	if (command.length() > 0) {
		std::string result =
				pythonInterpreter.interpretString(command.toStdString());
	}
	placeNewPrompt(endIsVisible);
}

void PythonConsoleWindow::placeNewPrompt(bool bMakeVisible)
{
	plainTextEdit->moveCursor(QTextCursor::End);
	plainTextEdit->insertPlainText(prompt);
	if (bMakeVisible)
		plainTextEdit->ensureCursorVisible();
	plainTextEdit->moveCursor(QTextCursor::End);
}

QString PythonConsoleWindow::getCurrentCommand()
{
	QTextCursor cursor = plainTextEdit->textCursor();
	cursor.movePosition(QTextCursor::StartOfLine);
	// Skip over the prompt, and begin the selection
	cursor.movePosition(QTextCursor::Right,
			QTextCursor::MoveAnchor, promptLength);
	// End selection at end of line
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	QString command = cursor.selectedText();
	// TODO - restore any previous selection
	cursor.clearSelection();
	return command;
}

} // namespace v3d
