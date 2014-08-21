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
#include "settings.hh"
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QDesktopServices>

Settings::Settings(): settings("AU", "RASMUS") {
	ui.setupUi(this);
	load();
}

void Settings::updateSettings() {
	ui.textColor->setText(QString("<span style=\"color: %1\">%1</span").arg(consoleTextColor.name()));
	ui.backgroundColor->setText(QString("<span style=\"color: %1\">%1</span").arg(consoleBackgroundColor.name()));
	ui.path->setText(path);
	ui.fontSize->setValue(consoleFont.pointSize());
	ui.fontName->setCurrentFont(consoleFont);
}

void Settings::selectFont() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, consoleFont, this);
	if (!ok) return;
	consoleFont = font;
	updateSettings();
}

void Settings::selectTextColor() {
	QColor c = QColorDialog::getColor(consoleTextColor, this);
	if (!c.isValid()) return;
	consoleTextColor=c;
	updateSettings();
}

void Settings::selectBackgroudColor() {
	QColor c = QColorDialog::getColor(consoleBackgroundColor, this);
	if (!c.isValid()) return;
	consoleBackgroundColor=c;
	updateSettings();
}

void Settings::selectPath() {
	QString path=QFileDialog::getExistingDirectory(this, "Path");
	if (path.isEmpty()) return;
	this->path = path;
	updateSettings();
}

void Settings::restoreDefaults() {
	settings.clear();
	load();
}

void Settings::clicked(QAbstractButton * button) {
	QDialogButtonBox::StandardButton standardButton = ui.buttonBox->standardButton(button);
	if (standardButton == QDialogButtonBox::RestoreDefaults) restoreDefaults();
}

void Settings::load() {
	consoleFont = settings.value("console/font", QFont("Courier", 12)).value<QFont>();
	consoleTextColor = settings.value("console/textColor", QColor(Qt::white)).value<QColor>();
	consoleBackgroundColor = settings.value("console/backgroundColor", QColor(Qt::black)).value<QColor>();
	path = settings.value("path",  QCoreApplication::applicationDirPath()+"/relations"
						  /*QDesktopServices::storageLocation(QDesktopServices::DataLocation)*/
		).value<QString>();
	updateSettings();
	emit visualUpdate(this);
}

void Settings::save() {
	settings.setValue("console/font", consoleFont);
	settings.setValue("console/textColor", consoleTextColor);
	settings.setValue("console/backgroundColor", consoleBackgroundColor);
	settings.setValue("path", path);
	emit visualUpdate(this);
}

void Settings::changeFont(QFont f) {
	consoleFont = f;
}

void Settings::changeFontSize(int i) {
	consoleFont.setPointSize(i);
}
