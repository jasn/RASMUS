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
#include <console.hh>
#include <QTextBlock>
#include <iostream>
#include <vector>
#include <stdlib/lib.h>
#include <QScrollBar>
#include <QDebug>
#include "settings.hh"
#include <vector>

#include <QPrinter>
#include <QMimeData>
#include <QPrintDialog>

class ConsolePrivate {
public:
	std::vector<QString> history;
	size_t currHistoryPosition;
	bool currentLineInsertedInHistory;
	bool incompleteState;
	Settings * settings;
	Console * outer;

	ConsolePrivate(Console * outer): 
		history(std::vector<QString>())
		, currHistoryPosition(0)
		, currentLineInsertedInHistory(false)
		, incompleteState(false)
		, outer(outer) {}

	void updateHistory() {
		QTextCursor c = outer->textCursor();
		
		QString currentLine = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
		
		history[history.size()-1-currHistoryPosition] = currentLine;
	}
	
	// update last line of last line in gui to show history[currentHistoryPosition]
	void rewriteCurrentLine() {
		QTextCursor c = outer->textCursor();
		
		int firstInLastBlock = c.document()->lastBlock().position()+4;
		int lastInLastBlock = firstInLastBlock + c.document()->lastBlock().text().size() - 4;
		c.setPosition(firstInLastBlock);
		c.setPosition(lastInLastBlock, QTextCursor::KeepAnchor);
		c.removeSelectedText();
		
		c.insertText(history[history.size()-currHistoryPosition-1]);
		outer->ensureCursorVisible();
	}

	void insertEmptyBlock() {
		QTextCursor c = outer->textCursor();
		c.movePosition(QTextCursor::End);
		c.insertBlock();
  
		if (incompleteState) c.insertHtml(
			QString("<span style=\"color: %1\">...</style>").arg(
				settings->color(Colors::consoleMessage).name()));
		else c.insertHtml(
			QString("<span style=\"color: %1\">&gt;&gt;&gt;</style>").arg(
				settings->color(Colors::consoleMessage).name()));
		QTextCharFormat cf = outer->currentCharFormat();
		cf.clearForeground();
		outer->setCurrentCharFormat(cf);
		c = outer->textCursor();
		c.insertText(" ");
		c.document()->clearUndoRedoStacks();
		outer->gotoEnd();
	}

};

void Console::pasteContinue() {
	QTextCursor c = textCursor();
	pasteActive = true;
	int index = pasteIndex;
	if (pasteBuffer[index].startsWith(">>> ") || pasteBuffer[index].startsWith("... ")) {
		pasteBuffer[index].remove(0, 4);
	}
	c.insertText(pasteBuffer[index]);
	QString tmp = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
	if (index+1 == pasteBuffer.size()) {
		pasteActive = false;
		return;
	}
	++pasteIndex;
	runLine(tmp);	
}

void Console::keyPressEvent(QKeyEvent *e) {
	if (e->key() == Qt::Key_Control ||
		e->key() == Qt::Key_Alt ||
		e->key() == Qt::Key_Shift ||
		(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_C)) {
		QPlainTextEdit::keyPressEvent(e);
		return;
	}

	if (isReadOnly()) {
		if (e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier)
			doCancel();
		return;
	}
	QTextCursor c = textCursor();
	
	int firstInLastBlock = c.document()->lastBlock().position() + 4;

	if (c.selectionStart() < firstInLastBlock)
		gotoEnd();

	if (d->history.size() == 0) d->history.push_back(QString::fromStdString(""));

	switch (e->key()) {
	case Qt::Key_Escape:
		if (d->incompleteState) {
			// do stuff.
			doCancel();
		}
		break;
	case Qt::Key_D:
		if (e->modifiers() == Qt::ControlModifier) {
			emit quit();
			break;
		}
		QPlainTextEdit::keyPressEvent(e);
		d->updateHistory();
		break;
	case Qt::Key_Home:
		c.setPosition(firstInLastBlock);
		setTextCursor(c);
		break;
	case Qt::Key_Backspace:
		if (c.positionInBlock() > 4) 
			QPlainTextEdit::keyPressEvent(e);
		break;
	case Qt::Key_Left:
		if (c.positionInBlock() > 4)
			QPlainTextEdit::keyPressEvent(e);
		break;	
	case Qt::Key_Right:
		QPlainTextEdit::keyPressEvent(e);
		break;
	case Qt::Key_Up:
		if (d->history.size()-1 == d->currHistoryPosition) break;
		++d->currHistoryPosition;
		d->rewriteCurrentLine();
		break;
	case Qt::Key_Down:
		if (d->currHistoryPosition == 0) break;
		--d->currHistoryPosition;
		d->rewriteCurrentLine();
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
		if (c.document()->lastBlock().text().size() <= 4) {
			gotoEnd();
			if (!d->incompleteState) {
				d->insertEmptyBlock();
			} else {
				incomplete();
			}
		} else {
			QString tmp = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
			runLine(tmp);
		}
		break;
      
	default:
		QPlainTextEdit::keyPressEvent(e);
		d->updateHistory();
	}

}

void Console::runLine(const QString &tmp) {
	rm_clearAbort();
	emit run(tmp);
	d->currHistoryPosition = 0;
	d->history[d->history.size()-1] = tmp;
	d->history.push_back(QString::fromStdString(""));

	gotoEnd();
}

void Console::incomplete() {
	d->incompleteState = true;
	d->insertEmptyBlock();
	if (pasteActive) pasteContinue();
}

void Console::complete() {
	d->incompleteState = false;
	d->insertEmptyBlock();
	if (pasteActive) pasteContinue();
}

void Console::display(QString block) {
	QTextCursor c = textCursor();
	c.movePosition(QTextCursor::End);
	c.insertText("\n");
	c.insertHtml(block);
	gotoEnd();
}

void Console::visualUpdate(Settings *s) {
	setFont(s->font(Fonts::console));
	QPalette p(palette());
	p.setColor(QPalette::Text, s->color(Colors::consoleText));
	p.setColor(QPalette::Base, s->color(Colors::consoleBackground));
	setPalette(p);
	d->settings = s;
}

void Console::doCancel() {
	if (isReadOnly()) rm_abort();
	else emit cancel();
}

void Console::bussy(bool bussy) {
	setReadOnly(bussy);
}

void Console::gotoEnd() {
	QTextCursor c = textCursor();
	c.clearSelection();
	c.movePosition(QTextCursor::End);
	setTextCursor(c);
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	ensureCursorVisible();
}

void Console::doPrintConsole() {
	QPrinter *printer = new QPrinter();
	QPrintDialog *pdlg = new QPrintDialog(printer, this);
	pdlg->setWindowTitle(tr("Print Console"));

	if (pdlg->exec() != QDialog::Accepted) return;

	print(printer);
}

void Console::insertFromMimeData(const QMimeData *source) {

	if (!source->hasText()) return;

	gotoEnd();
	QString txt = source->text();
	pasteBuffer.clear();
	pasteBuffer = txt.split("\n");
	pasteIndex = 0;
	if (pasteBuffer.size() > 0) {
		pasteContinue();
	}

}

Console::Console(QWidget * parent)
	: QPlainTextEdit(parent), pasteBuffer(QStringList()), pasteActive(false), pasteIndex(0) {
	d = new ConsolePrivate(this);
	d->history.push_back(QString(""));
}

Console::~Console() {
	delete d;
}
