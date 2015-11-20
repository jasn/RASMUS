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

/**
 * Texts that are visible somewhere in the UI.
 * These are used for customizing the font used to display them.
 */
enum class Fonts {
	console,
	editor,
	relation
};

/**
 * Objects that are visible somewhere in the UI which we want to
 * be able to customize the color.
 * E.g. backgrounds or texts.
 */
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

/**
 * This class inherits from QDialog and draws the Settings window opened through
 * Edit->Preferences.
 */
class Settings: public QDialog {
	Q_OBJECT
public:
	/**
	 * Constructor sets up the ui.
	 */
	Settings();

	/**
	 * @return the current path used to locate rasmus relations.
	 */
	QString path() const;

	/**
	 * @param f is a text type visisble in ui somewhere
	 * @return the QFont used for f
	 */
	QFont font(Fonts font) const;

	/**
	 * @param c is a Colors which refers to a a colored background/text/etc
	 * @param c refers to the object whose color we are interested in.
	 * @return the QColor associated with the type @param c.
	 * E.g. returns the QColor of the consoleText.
	 */
	QColor color(Colors color) const;
public slots:
	/**
	 * Opens a File Dialog to determine where rasmus relation files are located.
	 */
	void selectPath();
	/**
	 * This slot receives a signal when the Settings dialog is closed via 'Ok' button or
	 * when 'Apply' is clicked.
	 * The function stores settings.
	 */
	void save();

	/**
	 * This slot receives a signal when the Settings dialog is closed via closing the window.
	 * The function loads the stored settings.
	 */
	void load();

	/**
	 * Resets the settings to standard values.
	 */
	void restoreDefaults();
signals:
	/**
	 * Signal emitted whenever settings change.
	 */
	void visualUpdate(Settings *);
private:
	SettingsPrivate * d; /**< This is a container for stored data as well as some implementational details.. */
};

#endif //__SRC_GUI_SETTINGS_H__
