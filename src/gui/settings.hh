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

#ifndef __SRC_GUI_SETTINGS_H__
#define __SRC_GUI_SETTINGS_H__

#include <QDialog>
#include <QFont>
#include <QColor>

class SettingsPrivate;
class QAbstractButton;

enum class Fonts {
	console,
	editor,
	relation
};

enum class Colors {
	consoleText,
	consoleBackground,
	consoleMessage,
	consoleError,
	consoleWarning,
	consoleCode,
	editorNormal,
	editorBackground,
	editorKeyword,
	editorWarning,
	editorError,
	editorText,
	editorComment,
	relationText,
	relationBackground,
};

class Settings: public QDialog {
	Q_OBJECT
public:
	Settings();
	QString path() const;
	QFont font(Fonts font) const;
	QColor color(Colors color) const;
public slots:
	void selectPath();
	void save();
	void load();
	void restoreDefaults();
	void clicked(QAbstractButton * button);
signals:
	void visualUpdate(Settings *);
private:
	SettingsPrivate * d;
};

#endif //__SRC_GUI_SETTINGS_H__
