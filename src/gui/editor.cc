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

Editor::Editor(Settings * settings) {
	ui.setupUi(this);
	ui.edit->highlighter = new Highlighter(ui.edit->document());
	dirty(false);
	setAttribute(Qt::WA_DeleteOnClose);
	visualUpdate(settings);
}

void Editor::visualUpdate(Settings * s) {
	settings = s;
	ui.edit->setFont(s->font(Fonts::editor));
	QPalette p(palette());
	p.setColor(QPalette::Text, s->color(Colors::editorNormal));
	p.setColor(QPalette::Base, s->color(Colors::editorBackground));
	ui.edit->setPalette(p);
	static_cast<Highlighter*>(ui.edit->highlighter)->updateSettings(s);
}

Editor::Editor(Settings * settings, QString path, QString content): path(path) {
	ui.setupUi(this);
	ui.edit->highlighter = new Highlighter(ui.edit->document());
	ui.edit->document()->setPlainText(content);
	dirty(false);
	setAttribute(Qt::WA_DeleteOnClose);
	visualUpdate(settings);
}

void Editor::dirty(bool d) {
	if (path.filePath().isEmpty()) 
		setWindowTitle((d?"*":"")+QString("New File"));
	else
		setWindowTitle((d?"*":"")+path.fileName());

}

void Editor::closeEvent(QCloseEvent *event) {
	if (!ui.edit->document()->isUndoAvailable()) {
		event->accept();
		return;
	}

	QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Save?"), tr("Save document before closing?"),
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
	emit runContent(path.fileName(), ui.edit->toPlainText());
}

bool Editor::save() {
	if (path.filePath().isEmpty()) {
		return saveAs();
	}
	QFile file(path.filePath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", "Unable to upen file " + path.filePath() + " for writing");
		return false;
	}
	ui.edit->document()->clearUndoRedoStacks();
	file.write(ui.edit->toPlainText().toUtf8());
	file.close();
	return true;
}

bool Editor::saveAs() {
	QString p = QFileDialog::getSaveFileName(this, tr("Save As"), path.filePath(), tr("RASMUS Files (*.rm)"));
	if (p.isEmpty()) return false;
	path.setFile(p);
	if (!save()) return false;
	dirty(false);
	return true;
}

