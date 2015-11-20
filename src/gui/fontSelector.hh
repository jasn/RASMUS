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
#ifndef __GUI_FONT_SELECTOR_HH__
#define __GUI_FONT_SELECTOR_HH__

#include <QWidget>
#include <QFont>

class FontSelectorPrivate;

/**
 * In the settings window one can adjust the font settings.
 * This class is responsible for the forms used for adjusting these settings.
 * This class is responsible for maintaining which font is currently in use.
 * The form contains two fields: a font name and a size.
 */
class FontSelector: public QWidget {
	Q_OBJECT
public:
	/**
	 * Instantiates the form.
	 * @param parent is the settings window.
	 */
	FontSelector(QWidget * parent);

	/**
	 * Destructor. Is destructed when the the settings window is closed.
	 */
	~FontSelector();

	/**
	 * Gets the associated font for this form.
	 */
	QFont getFont();
public slots:
	/**
	 * Called whenever the font size has changed.
	 */
	void setFontSize(int size);

	/**
	 * Called whenever the font face has changed.
	 */
	void setFont(QFont font);

	/**
	 * Called when '...' is clicked in this font form.
	 * Opens a dialog for choosing font face, size, and style.
	 */
	void selectFont();
signals:
	void sizeChanged(int size);
	void fontChanged(QFont font);
private:
	FontSelectorPrivate * d; /**< Encapsulates the data */
};

#endif //__GUI_FONT_SELECTOR_HH__
