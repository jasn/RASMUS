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

class Editor: public QMainWindow {
	Q_OBJECT
public:
	Editor(Settings * settings);
	Editor(Settings * settings, QString path, QString content);
	~Editor();
	void closeEvent(QCloseEvent *event);
public slots:
	void dirty(bool dirty);
	void showAbout();
	void run();
	bool save();
	bool saveAs();
	void visualUpdate(Settings *);
	void doPrintEditor();
signals:
	void runContent(QString name, QString content);
private:
	EditorPrivate * d;
};

#endif //__SRC_GUI_EDITOR_H__
