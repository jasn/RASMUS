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
#ifndef __GUI_COLOR_SELECTOR_HH__
#define __GUI_COLOR_SELECTOR_HH__
#include <QWidget>
#include <QColor>

class ColorSelectorPrivate;
/**
 * In Edit->Preferences one can set up the colors to use for various texts.
 * Each text whose color can be customized is represented visually with this
 * widget. See colorSelector.ui for the looks.
 */
class ColorSelector: public QWidget {
	Q_OBJECT
public:
	/**
	 * @param is the Preferences Window.
	 */
	ColorSelector(QWidget * parent);
	~ColorSelector();

	/**
	 * @return the color of the associated text.
	 */
	QColor getColor();
public slots:
	/**
	 * When this slot receives a signal it opens a QColorDialog for the user to
	 * select a color. Currently this slot is connected to the clicked() event
	 * of the button in colorSelector.ui.
	 */
	void selectColor();

	/**
	 * Once a color has been chosen, this slot saves it and announces that a
	 * color has changed.
	 */
	void setColor(QColor);
signals:

	/**
	 * Signal used to announce a change of color.
	 * @param c is the new color.
	 */
	void colorChanged(QColor c);
private:
	ColorSelectorPrivate * d; /**< QT pattern for binary compatibility */
};


#endif //__GUI_COLOR_SELECTOR_HH__
