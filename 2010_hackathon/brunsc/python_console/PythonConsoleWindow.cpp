/*
 * pythonConsole.cpp
 *
 *  Created on: Dec 18, 2010
 *      Author: cmbruns
 */

#include "PythonConsoleWindow.h"
#include <iostream>
#include <QTextBlock>
#include <QtGui/QClipboard>
#include <QTime>
#include <QMessageBox>
#include <QThread>

namespace bp = boost::python;
using namespace std;

static QTime performanceTimer;

PythonConsoleWindow::PythonConsoleWindow(QWidget *parent)
		: QMainWindow(parent),
		  prompt(">>> "),
		  promptLength(prompt.length()),
		  multilineCommand(""),
		  commandRing(50) // remember latest 50 commands
{
	setupUi(this);
	setupMenus();

	setWindowIcon(QIcon(":/icons/python.png"));

#ifndef QT_NO_CLIPBOARD
    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
            this, SLOT(onClipboardDataChanged()) );
#endif
    connect( plainTextEdit, SIGNAL(selectionChanged()),
            this, SLOT(onSelectionChanged()) );

	// Run python command after user presses return
	plainTextEdit->installEventFilter(this);
	plainTextEdit->viewport()->installEventFilter(this); // to get mousePressed
	connect( this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()) );

	// Don't let cursor leave the editing region.
	connect( plainTextEdit, SIGNAL(cursorPositionChanged ()),
			this, SLOT(onCursorPositionChanged()) );

	plainTextEdit->setWordWrapMode(QTextOption::WrapAnywhere);

	// Print intro text at top of console.
	plainTextEdit->appendPlainText("Welcome to the V3D python console!");
	// This header text is intended to look just like the standard python banner.
	std::string pyVersion("Python ");
	pyVersion += Py_GetVersion();
	pyVersion += " on ";
	pyVersion += Py_GetPlatform();
	plainTextEdit->appendPlainText(pyVersion.c_str());

	plainTextEdit->appendPlainText(
			"Type \"help\", \"copyright\", \"credits\" or "
			"\"license()\" for more information.");

	plainTextEdit->appendPlainText(""); // need new line for prompt
	placeNewPrompt(true);

	// Make cursor about the size of a letter
	int cursorSize = QFontMetrics(plainTextEdit->font()).width("m");
    if (cursorSize > 0)
    		plainTextEdit->setCursorWidth(cursorSize);
    else
    		plainTextEdit->setCursorWidth(1);

	pythonInterpreter = new PythonInterpreter();
	connect(pythonInterpreter, SIGNAL(commandComplete()), this, SLOT(onCommandComplete()));
	connect(pythonInterpreter, SIGNAL(incompleteCommand(QString)), this, SLOT(onIncompleteCommand(QString)));
	connect(this, SIGNAL(commandIssued(QString)), pythonInterpreter, SLOT(interpretLine(QString)));
	connect(pythonInterpreter, SIGNAL(outputSent(QString)), this, SLOT(onOutput(QString)));
	// QThread* pythonThread = new QThread(this); // Create a separate thread for running python
	// pythonInterpreter->moveToThread(pythonThread);
	// pythonThread->start();
}

void PythonConsoleWindow::onOutput(QString msg) {
	plainTextEdit->moveCursor(QTextCursor::End);
	plainTextEdit->insertPlainText( msg );
}

void PythonConsoleWindow::onClipboardDataChanged()
{
    // cerr << "Data changed" << endl;
    // emit pasteAvailable(plainTextEdit->canPaste()); // slow
}

void PythonConsoleWindow::setupMenus()
{
    // Create menu actions

    actionUndo->setShortcuts(QKeySequence::Undo);
    actionUndo->setIcon(QIcon(":/icons/undo.png"));
    actionRedo->setShortcuts(QKeySequence::Redo);
    actionRedo->setIcon(QIcon(":/icons/redo.png"));
    actionCut->setShortcuts(QKeySequence::Cut);
    actionCut->setIcon(QIcon(":/icons/cut.png"));
    // actionCut->setIconVisibleInMenu(true);
    // enable cut
    connect( plainTextEdit, SIGNAL(copyAvailable(bool)),
            this, SLOT(onCopyAvailable(bool)) );
    connect( this, SIGNAL(cutAvailable(bool)),
            actionCut, SLOT(setEnabled(bool)) );
    actionCopy->setShortcuts(QKeySequence::Copy);
    actionCopy->setIcon(QIcon(":/icons/copy.png"));
    actionPaste->setShortcuts(QKeySequence::Paste);
    actionPaste->setIcon(QIcon(":/icons/paste.png"));
    connect( this, SIGNAL(pasteAvailable(bool)),
            actionPaste, SLOT(setEnabled(bool)) );
    actionSelect_All->setShortcuts(QKeySequence::SelectAll);
    actionSelect_All->setIcon(QIcon(":/icons/select-all.png"));
    connect( actionAbout, SIGNAL(triggered()),
            this, SLOT(about()) );
    // TODO - undo/redo on Zoom In/Out
    actionZoom_in->setShortcuts(QKeySequence::ZoomIn);
    actionZoom_in->setIcon(QIcon(":/icons/zoom-in.png"));
    connect(actionZoom_in, SIGNAL(triggered()),
            this, SLOT(zoomIn()) );
    actionZoom_out->setShortcuts(QKeySequence::ZoomOut);
    actionZoom_out->setIcon(QIcon(":/icons/zoom-out.png"));
    connect(actionZoom_out, SIGNAL(triggered()),
            this, SLOT(zoomOut()) );

	actionRun_recent->setIcon(QIcon(":/icons/open-recent.png"));
	actionRun_script->setIcon(QIcon(":/icons/run-script.png"));
}

void PythonConsoleWindow::zoomIn() {
    QFont newFont(plainTextEdit->font());
    int oldSize = newFont.pointSize();
    int newSize = int(1.03 * oldSize + 0.5) + 1;
    newFont.setPointSize(newSize);
    plainTextEdit->setFont(newFont);
	int cursorSize = QFontMetrics(plainTextEdit->font()).width("m");
    if (cursorSize > 0)
    		plainTextEdit->setCursorWidth(cursorSize);
    else
    		plainTextEdit->setCursorWidth(1);
}

void PythonConsoleWindow::zoomOut() {
    QFont newFont(plainTextEdit->font());
    int oldSize = newFont.pointSize();
    int newSize = int(oldSize / 1.03 + 0.5) - 1;
    if (newSize < 1) newSize = 1;
    newFont.setPointSize(newSize);
    plainTextEdit->setFont(newFont);
	int cursorSize = QFontMetrics(plainTextEdit->font()).width("m");
    if (cursorSize > 0)
    		plainTextEdit->setCursorWidth(cursorSize);
    else
    		plainTextEdit->setCursorWidth(1);
}

// When user presses <Return> key in text area, execute the python command
bool PythonConsoleWindow::eventFilter ( QObject * watched, QEvent * event )
{
    if (event->type() == QEvent::MouseButtonPress) {
        // On unix, we want to update cursor position on middle
        // button press, before deciding whether editing is possible.
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() == Qt::MiddleButton) {
            // cerr << "middle button" << endl;
            QTextCursor newCursor = plainTextEdit->
                    cursorForPosition(mouseEvent->pos());
            plainTextEdit->setTextCursor(newCursor);
        }
    }
    else if (event->type() == QEvent::KeyPress)
	{
        // performanceTimer.start();
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch(keyEvent->key())
        {
        // Use up and down arrows for history
        case Qt::Key_P:
            if (! (keyEvent->modifiers() & Qt::ControlModifier))
                break; // Only care about Ctrl-P
        case Qt::Key_Up:
            showPreviousCommand();
            return true;
            break;
        case Qt::Key_N:
            if (! (keyEvent->modifiers() & Qt::ControlModifier))
                break; // Only care about Ctrl-N
        case Qt::Key_Down:
            showNextCommand();
            return true;
            break;
        // Prevent left arrow from leaving editing area
        case Qt::Key_Left:
        case Qt::Key_Backspace:
            if ( (plainTextEdit->textCursor().positionInBlock() == promptLength)
            		&& cursorIsInEditingRegion(plainTextEdit->textCursor()) )
            {
            		return true; // no moving left into prompt with arrow key
            }
            break;
        // Trigger command execution with <Return>
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit returnPressed();
            return true; // Consume event.  We will take care of inserting the newline.
        }
        // cerr << "key press elapsed time = " << performanceTimer.elapsed() << " ms" << endl;
	}

	return QMainWindow::eventFilter(watched, event);
}

void PythonConsoleWindow::showPreviousCommand() {
    QString provisionalCommand = getCurrentCommand();
    QString previousCommand =
            commandRing.getPreviousCommand(provisionalCommand);
    if (previousCommand != provisionalCommand)
        replaceCurrentCommand(previousCommand);
}
void PythonConsoleWindow::showNextCommand() {
    QString provisionalCommand = getCurrentCommand();
    QString nextCommand =
            commandRing.getNextCommand(provisionalCommand);
    if (nextCommand != provisionalCommand)
        replaceCurrentCommand(nextCommand);
}

void PythonConsoleWindow::about() {
    QMessageBox::information( this,
            tr("About V3D python console."),
            tr("V3D python console\n"
               "By Christopher and Cami Bruns\n"
               "HHMI Janelia Farm Research Campus\n"
               "December 2010") );
}

void PythonConsoleWindow::setPrompt(const QString& newPrompt)
{
	prompt = newPrompt;
	promptLength = prompt.length();
}

void PythonConsoleWindow::executeCommand(const QString& command)
{
    plainTextEdit->moveCursor(QTextCursor::End);
    currentCommandStartPosition =
            plainTextEdit->textCursor().position();
    plainTextEdit->insertPlainText(command);
    onReturnPressed();
}

void PythonConsoleWindow::onReturnPressed()
{
    // Clear undo/redo buffer, we don't want prompts and output in there.
    plainTextEdit->setUndoRedoEnabled(false);
    // Scroll down after command, if and only if bottom is visible now.
    bool endIsVisible = plainTextEdit->document()->lastBlock().isVisible();

    QString command = getCurrentCommand();
    commandRing.addHistory(command);
    if (multilineCommand.length() > 0) {
       // multi-line command can only be ended with a blank line.
       if (command.length() == 0)
           command = multilineCommand; // execute it now
       else {
           multilineCommand = multilineCommand + command + "\n";
           command = ""; // skip execution until next time
       }
    }

    // Add carriage return, so output will appear on subsequent line.
    // (It would be too late if we waited for plainTextEdit
    //  to process the <Return>)
    plainTextEdit->moveCursor(QTextCursor::End);
    plainTextEdit->appendPlainText("");  // We consumed the key event, so we have to add the newline.

    if (command.length() > 0)
        emit commandIssued(command);
    else
        placeNewPrompt(endIsVisible);
}

void PythonConsoleWindow::onCommandComplete()
{
	multilineCommand = "";
    bool endIsVisible = plainTextEdit->document()->lastBlock().isVisible();
	setPrompt(">>> ");
	placeNewPrompt(endIsVisible);
}

void PythonConsoleWindow::onIncompleteCommand(QString partialCmd)
{
	multilineCommand = partialCmd + "\n";
    bool endIsVisible = plainTextEdit->document()->lastBlock().isVisible();
	setPrompt("... ");
	placeNewPrompt(endIsVisible);
}

void PythonConsoleWindow::onCopyAvailable(bool bCopyAvailable)
{
    if (! bCopyAvailable)
        emit cutAvailable(false);
    else if (cursorIsInEditingRegion(plainTextEdit->textCursor()))
        emit cutAvailable(true);
    else
        emit cutAvailable(false);
}

bool PythonConsoleWindow::cursorIsInEditingRegion(const QTextCursor& cursor)
{
    // Want to be to the right of the prompt...
    if (cursor.positionInBlock() < promptLength)
        return false;
    // ... and in the final line.
    if (cursor.blockNumber() != plainTextEdit->blockCount() - 1)
        return false;
    if (cursor.anchor() != cursor.position()) {
        // Anchor might be outside of editing region
        QTextCursor anchorCursor(cursor);
        anchorCursor.setPosition(cursor.anchor());
        if (anchorCursor.positionInBlock() < promptLength)
            return false;
        if (anchorCursor.blockNumber() != plainTextEdit->blockCount() - 1)
            return false;
    }
    return true;
}

void PythonConsoleWindow::onSelectionChanged()
{
    QTextCursor cursor = plainTextEdit->textCursor();
    bool bReadOnly = ! cursorIsInEditingRegion(cursor);
    if (bReadOnly != plainTextEdit->isReadOnly())
        plainTextEdit->setReadOnly(bReadOnly);
}

void PythonConsoleWindow::onCursorPositionChanged()
{
    // performanceTimer.start();
	cerr << "Cursor moved" << endl;
    // Don't allow editing outside the editing area.
	QTextCursor currentCursor = plainTextEdit->textCursor();
	bool bReadOnly;

    if (cursorIsInEditingRegion(currentCursor)) {
        // This is a good spot.  Within the editing area
        latestGoodCursorPosition = currentCursor;
        bReadOnly = false;
    }
    else {
        bReadOnly = true;
    }
    if (bReadOnly != plainTextEdit->isReadOnly())
        plainTextEdit->setReadOnly(bReadOnly);

    // cerr << "cursor position elapsed time1 = " << performanceTimer.elapsed() << " ms" << endl;
    if(bReadOnly) {
        emit pasteAvailable(false);
        emit cutAvailable(false);
    }
    else {
        // Performance problem with canPaste() method.
        // plainTextEdit->canPaste(); // slow ~120 ms
        // emit pasteAvailable(plainTextEdit->canPaste()); // slow
        // emit pasteAvailable(!QApplication::clipboard()->text().isEmpty());
        // QApplication::clipboard()->text().isEmpty(); // slow ~ 120 ms
        emit pasteAvailable(true); // whatever...
    }
    // cerr << "cursor position elapsed time2 = " << performanceTimer.elapsed() << " ms" << endl;
}

void PythonConsoleWindow::placeNewPrompt(bool bMakeVisible)
{
	plainTextEdit->setUndoRedoEnabled(false); // clear undo/redo buffer
	plainTextEdit->moveCursor(QTextCursor::End);
	plainTextEdit->insertPlainText(prompt);
	if (bMakeVisible) {
		plainTextEdit->ensureCursorVisible();
		// cerr << "make visible" << endl;
	}
	plainTextEdit->moveCursor(QTextCursor::End);
	latestGoodCursorPosition = plainTextEdit->textCursor();
	currentCommandStartPosition = latestGoodCursorPosition.position();
	// Start undo/redo, just for user typing, not for computer output
	plainTextEdit->setUndoRedoEnabled(true);
}

QString PythonConsoleWindow::getCurrentCommand()
{
	QTextCursor cursor = plainTextEdit->textCursor();
	cursor.setPosition(currentCommandStartPosition,
	        QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	QString command = cursor.selectedText();
	cursor.clearSelection();
	return command;
}

//Replace current command with a new one
void PythonConsoleWindow::replaceCurrentCommand(const QString& newCommand)
{
    QTextCursor cursor = plainTextEdit->textCursor();
    cursor.setPosition(currentCommandStartPosition,
            QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End,
            QTextCursor::KeepAnchor);
    cursor.insertText(newCommand);
}
