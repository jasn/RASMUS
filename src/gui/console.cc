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
#include "settings.hh"

void Console::visualUpdate(Settings *s) {

	setFont(s->consoleFont);
	QPalette p(palette());
	p.setColor(QPalette::Text, s->consoleTextColor);
	p.setColor(QPalette::Base, s->consoleBackgroundColor);
  
	setPalette(p);

}

void Console::updateHistory() {
	QTextCursor c = textCursor();

	QString currentLine = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
  
	history[history.size()-1-currHistoryPosition] = currentLine;

}

// update last line of last line in gui to show history[currentHistoryPosition]
void Console::rewriteCurrentLine() {
	QTextCursor c = textCursor();
  
	int firstInLastBlock = c.document()->lastBlock().position()+4;
	int lastInLastBlock = firstInLastBlock + c.document()->lastBlock().text().size() - 4;
	c.setPosition(firstInLastBlock);
	c.setPosition(lastInLastBlock, QTextCursor::KeepAnchor);
	c.removeSelectedText();

	c.insertText(history[history.size()-currHistoryPosition-1]);
	ensureCursorVisible();

}

void Console::doCancel() {
	if (isReadOnly()) rm_abort();
	else emit cancel();
}

void Console::gotoEnd() {
	QTextCursor c = textCursor();
	c.clearSelection();
	c.movePosition(QTextCursor::End);
	setTextCursor(c);
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	ensureCursorVisible();
}


void Console::keyPressEvent(QKeyEvent *e) {
	if (isReadOnly()) {
		if (e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier)
			doCancel();
		if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier) 
			doCancel();
		return;
	}
	QTextCursor c = textCursor();
    
	int firstInLastBlock = c.document()->lastBlock().position() + 4;

	if (c.selectionStart() < firstInLastBlock)
		gotoEnd();

	if (history.size() == 0) history.push_back(QString::fromStdString(""));

	switch (e->key()) {
	case Qt::Key_Escape:
		if (incompleteState) {
			// do stuff.
			doCancel();
		}
		break;
	case Qt::Key_C:
		if (e->modifiers() == Qt::ControlModifier) {
			doCancel();
			break;
		}
		QPlainTextEdit::keyPressEvent(e);
		updateHistory();
		break;
	case Qt::Key_D:
		if (e->modifiers() == Qt::ControlModifier) {
			emit quit();
			break;
		}
		QPlainTextEdit::keyPressEvent(e);
		updateHistory();
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
		if (history.size()-1 == currHistoryPosition) break;
		++currHistoryPosition;
		rewriteCurrentLine();
		break;
	case Qt::Key_Down:
		if (currHistoryPosition == 0) break;
		--currHistoryPosition;
		rewriteCurrentLine();
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
		if (c.document()->lastBlock().text().size() <= 4) {
			gotoEnd();
			if (!incompleteState) {
				insertEmptyBlock();
			} else {
				incomplete();
			}
		} else {
			QString tmp = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
			rm_clearAbort();
			emit run(tmp);
			currHistoryPosition = 0;
			history[history.size()-1] = tmp;
			history.push_back(QString::fromStdString(""));
		
			gotoEnd();
		}
		break;
      
	default:
		QPlainTextEdit::keyPressEvent(e);
		updateHistory();
	}

}

void Console::bussy(bool bussy) {
	setReadOnly(bussy);
}

void Console::incomplete() {
	incompleteState = true;
	insertEmptyBlock();
}

void Console::complete() {
	incompleteState = false;
	insertEmptyBlock();
}

void Console::insertEmptyBlock() {
	QTextCursor c = textCursor();
	c.movePosition(QTextCursor::End);
	c.insertBlock();
  
	if (incompleteState) c.insertHtml("<span style=\"color: #7070FF\">...</style>");
	else c.insertHtml("<span style=\"color: #7070FF\">&gt;&gt;&gt;</style>");
	QTextCharFormat cf = currentCharFormat();
	cf.clearForeground();
	setCurrentCharFormat(cf);
	c = textCursor();
	c.insertText(" ");
	c.document()->clearUndoRedoStacks();
	gotoEnd();
}

void Console::display(QString block) {
	QTextCursor c = textCursor();
	c.movePosition(QTextCursor::End);
	c.insertText("\n");
	c.insertHtml(block);
	gotoEnd();
}

Console::Console(QWidget * parent)
	: QPlainTextEdit(parent)
	, history(std::vector<QString>())
	, currHistoryPosition(0)
	, currentLineInsertedInHistory(false)
	, incompleteState(false) {}
