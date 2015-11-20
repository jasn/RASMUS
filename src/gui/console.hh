// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>
#ifndef __SRC_GUI_CONSOLE_H__
#define __SRC_GUI_CONSOLE_H__

#include <QPlainTextEdit>

class Settings;
class ConsolePrivate;

/**
 * This is the console that is seen in the main window of the RASMUS GUI.
 * In this console one can execute rasmus code.
 * The console implments history among other things and in general behaves
 * like one would expect a terminal to behave.
 *
 * If an incomplete RASMUS expression is typed, it asks for more input before
 * going back to the default state.
 */
class Console: public QPlainTextEdit {
	Q_OBJECT
public:
	/**
	 * Constructor.
	 * @param parent is the MainWindow.
	 */
	Console(QWidget * parent);

	/**
	 * Destructor.
	 */
	~Console();

	/**
	 * Event triggered when pressing keys while this console is active.
	 */
	void keyPressEvent(QKeyEvent *e);

public slots:
	/**
	 * This slot receives a signal if incomplete code was executed.
	 * This allows us to ask for more input to be executed.
	 */
	void incomplete();

	/**
	 * This slot receives a signal if the last piece of code executed
	 * was 'complete', i.e. the code did not encounter unexpected EOF.
	 */
	void complete();

	/**
	 * This slot receives a signal when something needs to be displayed in
	 * the console. The msg is an html string which could be various things.
	 * It could be showing an error from the last executed expression, or
	 * the result of the last expression.
	 * @param msg html encoded msg to display in the terminal.
	 */
	void display(QString msg);

	/**
	 * This slot receives a signal whenever settings are updated.
	 * I.e. if a text changes color, size, or font.
	 * @param s the updated Settings.
	 */
	void visualUpdate(Settings *s);

	/**
	 * This slot receives a signal when Edit->Cancel is triggered.
	 * The purpose is to stop the current rasmus computation.
	 */
	void doCancel();

	/**
	 * When a computation starts, the console is set to a 'read-only' state.
	 * This slot receives a signal when when the Interpreter starts a computation
	 * and when it ends a computation.
	 * @param b set to true means the console will be put in the 'read-only' state
	 * and if b is set to false, then the console will be reverted to read/write state.
	 */
	void bussy(bool b);

	/**
	 * The purpose is to move the cursor in the console to the bottom and end.
	 * This is to emulate the behaviour of a meaningful terminal.
	 * Ie. we are not allowed to modify the history as it is seen on the screen.
	 * This slot receives a signal from many different places.
	 * E.g. when computations finish they produce a result which needs to be shown,
	 * then this function is called, to move the cursor to the bottom and then
	 * the computation result can be shown.
	 */
	void gotoEnd();

	/**
	 * This slot is activated when triggering File->Print
	 * The purpose is to print the contents of the console.
	 */
	void doPrintConsole();

	/**
	 * This slot is triggered when pasting code into the rasmus console.
	 * The slot is triggered from the Qt framework.
	 */
	void insertFromMimeData(const QMimeData *) override;
signals:

	/**
	 * This signal is emitted by runLine(const QString) and it is
	 * received by the Interpreter::run(QString) slot.
	 * @param line is RASMUS code that needs to be executed.
	 */
	void run(QString line);

	/**
	 * This signal is emitted by doCancel() and its purpose is
	 * to stop the current computation.
	 * The signal is received by the Interpreter::cancel() slot.
	 */
	void cancel();

	/**
	 * This signal is emitted when the user sends a 'quit' signal
	 * via Ctrl-D in the console.
	 * The signal is received by the MainWindow::close() slot.
	 */
	void quit();

private:

	/**
	 * Used to maintain history of the terminal as well as
	 * signal the rasmus::frontend::Interperter to run code.
	 */
	void runLine(const QString &tmp);

	/**
	 * Helper method used when pasting text into the console.
	 */
	void pasteContinue();

	QStringList pasteBuffer; /**< used for pasting multiple lines */
	bool pasteActive; /**< determines if we are currently in a 'pasting' state */
	int pasteIndex; /**< tells how much of pasteBuffer has been pasted */

	ConsolePrivate * d; /**< encapsulates for Console. This is Qt pattern. */
};


#endif //__SRC_GUI_CONSOLE_H__
