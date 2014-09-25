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
#include <QFileDialog>
#include <QSettings>
#include "ui_settings.h"
#include <QAbstractButton>
#include <QDebug>
#include <iostream>

class SettingsPrivate {
public:
	QSettings settings;
	QString path;
	Ui::Settings ui;

	SettingsPrivate(): settings("AU", "RASMUS") { }

	void setPath(QString path) {
		this->path = path;
		ui.path->setText(path);
	}
	
	void save() {
		settings.setValue("console/font", ui.consoleFont->getFont());
		settings.setValue("console/textColor", ui.consoleTextColor->getColor());
		settings.setValue("console/backgroundColor", ui.consoleBackgroundColor->getColor());
		settings.setValue("console/messageColor", ui.consoleMessageColor->getColor());
		settings.setValue("console/errorColor", ui.consoleErrorColor->getColor());
		settings.setValue("console/warningColor", ui.consoleWarningColor->getColor());
		settings.setValue("console/codeColor", ui.consoleCodeColor->getColor());

		settings.setValue("editor/font", ui.editorFont->getFont());
		settings.setValue("editor/normalColor", ui.editorNormalColor->getColor());
		settings.setValue("editor/backgroundColor", ui.editorBackgroundColor->getColor());
		settings.setValue("editor/keywordColor", ui.editorKeywordColor->getColor());
		settings.setValue("editor/warningColor", ui.editorWarningColor->getColor());
		settings.setValue("editor/errorColor", ui.editorErrorColor->getColor());
		settings.setValue("editor/textColor", ui.editorTextColor->getColor());
		settings.setValue("editor/commentColor", ui.editorCommentColor->getColor());
		
		settings.setValue("relation/font", ui.relationFont->getFont());
		settings.setValue("relation/textColor", ui.relationTextColor->getColor());
		settings.setValue("relation/backgroundColor", ui.relationBackgroundColor->getColor());
		settings.setValue("path", path);
	}
	
	void load() {
		ui.consoleFont->setFont(
			settings.value("console/font", QFont("Courier", 12)).value<QFont>());
		ui.consoleTextColor->setColor(
			settings.value("console/textColor", QColor("#DDDDDD")).value<QColor>());
		ui.consoleBackgroundColor->setColor(
			settings.value("console/backgourdColor", QColor(Qt::black)).value<QColor>());
		ui.consoleMessageColor->setColor(
			settings.value("console/messageColor", QColor("#7070FF")).value<QColor>());
		ui.consoleErrorColor->setColor(
			settings.value("console/errorColor", QColor(Qt::red)).value<QColor>());
		ui.consoleWarningColor->setColor(
			settings.value("console/warningColor", QColor(Qt::yellow)).value<QColor>());
		ui.consoleCodeColor->setColor(
			settings.value("console/codeColor", QColor(Qt::green)).value<QColor>());

		ui.editorFont->setFont(
			settings.value("editor/font", QFont("Courier", 12)).value<QFont>());
		ui.editorNormalColor->setColor(
			settings.value("editor/normalColor", QColor(Qt::black)).value<QColor>());
		ui.editorBackgroundColor->setColor(
			settings.value("editor/backgroundColor", QColor(Qt::white)).value<QColor>());
		ui.editorKeywordColor->setColor(
			settings.value("editor/keywordColor", QColor(Qt::blue)).value<QColor>());
		ui.editorWarningColor->setColor(
			settings.value("editor/warningColor", QColor(Qt::yellow)).value<QColor>());
		ui.editorErrorColor->setColor(
			settings.value("editor/errorColor", QColor(Qt::red)).value<QColor>());
		ui.editorTextColor->setColor(
			settings.value("editor/textColor", QColor(Qt::green)).value<QColor>());
		ui.editorCommentColor->setColor(
			settings.value("editor/commentColor", QColor(Qt::red)).value<QColor>());

		QWidget w;
		ui.relationFont->setFont(
			settings.value("relation/font", w.font()).value<QFont>());
		ui.relationTextColor->setColor(
			settings.value("relation/textColor", QColor(Qt::black)).value<QColor>());
		ui.relationBackgroundColor->setColor(
			settings.value("relation/backgroundColor", QColor(Qt::white)).value<QColor>());

		QString path=QCoreApplication::applicationDirPath()+"/relations";		
		#ifdef __APPLE__
		if (!QDir(path).exists()) path=QCoreApplication::applicationDirPath()+"/../../../relations";
		#endif

		setPath(
			settings.value("path",  QDir(path).absolutePath()).value<QString>());
	}

};

Settings::Settings() {
	d = new SettingsPrivate();
	d->ui.setupUi(this);
	QPushButton * restoreButton = d->ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
	connect(restoreButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
	QPushButton * applyButton = d->ui.buttonBox->button(QDialogButtonBox::Apply);
	connect(applyButton, SIGNAL(clicked()), this, SLOT(save()));
	load();
}

void Settings::selectPath() {
	QString path=QFileDialog::getExistingDirectory(this, "Path");
	if (path.isEmpty()) return;
	d->setPath(path);
}

void Settings::restoreDefaults() {
	d->settings.clear();
	d->load();
}

void Settings::load() {
	d->load();
	emit visualUpdate(this);
}

void Settings::save() {
	d->save();
	emit visualUpdate(this);
}

QColor Settings::color(Colors c) const {
	switch (c) {
	case Colors::consoleText: return d->ui.consoleTextColor->getColor();
	case Colors::consoleBackground: return d->ui.consoleBackgroundColor->getColor();
	case Colors::consoleMessage: return d->ui.consoleMessageColor->getColor();
	case Colors::consoleError: return d->ui.consoleErrorColor->getColor();
	case Colors::consoleWarning: return d->ui.consoleWarningColor->getColor();
	case Colors::consoleCode: return d->ui.consoleCodeColor->getColor();
	case Colors::editorNormal: return d->ui.editorNormalColor->getColor();
	case Colors::editorBackground: return d->ui.editorBackgroundColor->getColor();
	case Colors::editorKeyword: return d->ui.editorKeywordColor->getColor();
	case Colors::editorWarning: return d->ui.editorWarningColor->getColor();
	case Colors::editorError: return d->ui.editorErrorColor->getColor();
	case Colors::editorText: return d->ui.editorTextColor->getColor();
	case Colors::editorComment: return d->ui.editorCommentColor->getColor();
	case Colors::relationBackground: return d->ui.relationBackgroundColor->getColor();
	case Colors::relationText: return d->ui.relationTextColor->getColor();
	}
	__builtin_unreachable();
}

QFont Settings::font(Fonts f) const {
	switch (f) {
	case Fonts::console: return d->ui.consoleFont->getFont();
	case Fonts::editor: return d->ui.editorFont->getFont();
	case Fonts::relation: return d->ui.relationFont->getFont();
	}
	__builtin_unreachable();
}

QString Settings::path() const {
	return d->path;
}
