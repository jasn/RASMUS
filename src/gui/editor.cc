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
#include "editor.hh"
#include "highlighter.hh"
#include <QFileDialog>
#include <QMessageBox>
#include "help.hh"
#include "settings.hh"
#include "ui_editor.h"
#include <QFileInfo>

class EditorPrivate {
public:
	Ui::Editor ui;
	QFileInfo path;
	Settings * settings;
};

Editor::Editor(Settings * settings) {
	d = new EditorPrivate();
	d->ui.setupUi(this);
	d->ui.edit->highlighter = new Highlighter(d->ui.edit->document());
	dirty(false);
	setAttribute(Qt::WA_DeleteOnClose);
	visualUpdate(settings);
}

Editor::Editor(Settings * settings, QString path, QString content) {
	d = new EditorPrivate();
	d->path = path;
	
	d->ui.setupUi(this);
	d->ui.edit->highlighter = new Highlighter(d->ui.edit->document());
	d->ui.edit->document()->setPlainText(content);
	dirty(false);
	setAttribute(Qt::WA_DeleteOnClose);
	visualUpdate(settings);
}

Editor::~Editor() {
	delete d;
}

void Editor::visualUpdate(Settings * s) {
	d->settings = s;
	d->ui.edit->setFont(s->font(Fonts::editor));
	QPalette p(palette());
	p.setColor(QPalette::Text, s->color(Colors::editorNormal));
	p.setColor(QPalette::Base, s->color(Colors::editorBackground));
	d->ui.edit->setPalette(p);
	static_cast<Highlighter*>(d->ui.edit->highlighter)->updateSettings(s);
}

void Editor::dirty(bool dirt) {
	if (d->path.filePath().isEmpty()) 
		setWindowTitle((dirt?"*":"")+QString("New File"));
	else
		setWindowTitle((dirt?"*":"")+d->path.fileName());

}

void Editor::closeEvent(QCloseEvent *event) {
	if (!d->ui.edit->document()->isUndoAvailable()) {
		event->accept();
		return;
	}

	QMessageBox::StandardButton btn = QMessageBox::question(
		this, 
		tr("Save?"), 
		tr("Save document before closing?"),
		QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
	switch(btn) {
	case QMessageBox::Discard:
		event->accept();
		break;
	case QMessageBox::Save:		
		if (save())
			event->accept();
		else
			event->ignore();
		break;
	default:
		event->ignore();
		break;
	}
}


void Editor::showAbout() {
	::showAbout();
}

void Editor::run() {
	emit runContent(d->path.fileName(), d->ui.edit->toPlainText());
}

bool Editor::save() {
	if (d->path.filePath().isEmpty()) {
		return saveAs();
	}
	QFile file(d->path.filePath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", "Unable to upen file " + d->path.filePath() + " for writing");
		return false;
	}
	d->ui.edit->document()->clearUndoRedoStacks();
	file.write(d->ui.edit->toPlainText().toUtf8());
	file.close();
	return true;
}

bool Editor::saveAs() {
	QString p = QFileDialog::getSaveFileName(
		this, tr("Save As"), d->path.filePath(), 
		tr("RASMUS Files (*.rm)"));
	if (p.isEmpty()) return false;
	d->path.setFile(p);
	if (!save()) return false;
	dirty(false);
	return true;
}

