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

#ifndef __SRC_GUI_EDITOR_H__
#define __SRC_GUI_EDITOR_H__

#include <QMainWindow>

class Settings;
class EditorPrivate;

/**
 * The editor for rasmus code files.
 * The editor is a QMainWindow.
 */
class Editor: public QMainWindow {
	Q_OBJECT
public:
	/**
	 * Used when creating a new file.
	 * This sets up the ui and listens for chages in the Settings.
	 * @param settings is the Settings defined by the user.
	 */
	Editor(Settings * settings);

	/**
	 * Used when loading an existing file.
	 * @param settings is the Settings defiend by the user.
	 * @param path is a path to the file opened.
	 * @param content is the contents of the opened file.
	 */
	Editor(Settings * settings, QString path, QString content);

	/**
	 * Destrutor for the editor. Called when the editor window is closed,
	 * but after closeEvent has been called.
	 */
	~Editor();

	/**
	 * Called when closing the editor. Checks if there are unsaved changes and
	 * presents the user with a dialog in that case.
	 */
	void closeEvent(QCloseEvent *event);
public slots:

	void dirty(bool dirty);

	/**
	 * Received when triggering Help->About, executes showAbout() in helper.hh
	 */
	void showAbout();
	/**
	 * receives the 'triggered()' event when 'File->Run' is triggered/clicked.
	 * emits the runContent(QString name, QString content) signal, where
	 * the name is a file name and content is the textual contents of the
	 * editor, i.e. RASMUS code.
	 */
	void run();
	/**
	 * Receives the signal triggered by File->Save (or its shortcut).
	 * Saves the current contes to the current file location or asks for a
	 * location if it has not been saved before.
	 */
	bool save();
	/**
	 * Receives the signal triggered by 'File->Save As' (or its shortcur).
	 * Queries the user for a file name to save the current contents to.
	 */
	bool saveAs();
	/**
	 * Whenever settings change, e.g. font or colors this slot receives a signal.
	 * The function updates the editor to reflect the changes.
	 */
	void visualUpdate(Settings *);

	/**
	 * This slot receives a signal when File->Print is triggered.
	 * The function opens a Print dialog window. The printout is
	 * the content of the editor.
	 */
	void doPrintEditor();
signals:

	/**
	 * The editor has a 'Run' functionality that executes the rasmus code
	 * currently written in the editor.
	 * When that button is clicked this signal is emitted.
	 * Technically the button first emits a 'triggered()' signal, which is
	 * received by the run() slot above, which in turn emits the runContent(..) signal
	 */
	void runContent(QString name, QString content);
private:
	EditorPrivate * d;
};

#endif //__SRC_GUI_EDITOR_H__
