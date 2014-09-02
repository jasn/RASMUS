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
#include "fontSelector.hh"
#include <QFontDialog>
#include "ui_fontSelector.h"

class FontSelectorPrivate {
public:
	Ui::FontSelector ui;
	QFont font;
};

FontSelector::FontSelector(QWidget * parent): QWidget(parent) {
	d = new FontSelectorPrivate();
	d->font.setOverline(true);
	d->ui.setupUi(this);
}

FontSelector::~FontSelector() {
	delete d;
}

QFont FontSelector::getFont() {
	return d->font;
}

void FontSelector::setFontSize(int size) {
	QFont f=d->font;
	f.setPointSize(size);
	setFont(f);
}

void FontSelector::setFont(QFont font) {
	if (font == d->font) return;
	d->font = font;
	d->ui.size->setValue(font.pointSize());
	d->ui.name->setCurrentFont(font);
	emit fontChanged(font);
}

void FontSelector::selectFont() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, d->font, this);
	if (!ok) return;
	setFont(font);
}
