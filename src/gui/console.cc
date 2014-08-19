#include <console.hh>
#include <QTextBlock>
#include <iostream>
#include <vector>
#include <stdlib/lib.h>
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


void Console::keyPressEvent(QKeyEvent *e) {
  if (isReadOnly()) {
    if (e->key() == Qt::Key_Escape) 
      doCancel();
    return;
  }
    QTextCursor c = textCursor();
    
    int firstInLastBlock = c.document()->lastBlock().position() + 4;

    if (c.selectionStart() < firstInLastBlock) {
      c.clearSelection();
      c.movePosition(QTextCursor::End);
      setTextCursor(c);
    }

    if (history.size() == 0) history.push_back(QString::fromStdString(""));

    switch (e->key()) {
    case Qt::Key_Escape:
      if (incompleteState) {
	// do stuff.
	doCancel();
      }
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
	c.movePosition(QTextCursor::End);
	if (!incompleteState) {
	  insertEmptyBlock();
	} else {
	  incomplete();
	}
      } else {
	QString tmp = c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4);
	setReadOnly(true);
	rm_clearAbort();
	emit run(tmp);
	currHistoryPosition = 0;
	history[history.size()-1] = tmp;
	history.push_back(QString::fromStdString(""));
	
      }
      break;
      
    default:
      QPlainTextEdit::keyPressEvent(e);
      updateHistory();
    }

}

void Console::incomplete() {
  setReadOnly(false);
  incompleteState = true;
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertBlock();
  c.insertHtml("<span style=\"color: blue\">...</style>");

  QTextCharFormat cf = currentCharFormat();
  cf.clearForeground();
  setCurrentCharFormat(cf);
  c = textCursor();
  c.insertText(" ");
  c.document()->clearUndoRedoStacks();
  ensureCursorVisible();
}

void Console::complete() {
  setReadOnly(false);
  insertEmptyBlock();
  incompleteState = false;
}

void Console::insertEmptyBlock() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertBlock();
  c.insertHtml("<span style=\"color: blue\">&gt;&gt;&gt;</style>");
  QTextCharFormat cf = currentCharFormat();
  cf.clearForeground();
  setCurrentCharFormat(cf);
  c = textCursor();
  c.insertText(" ");
  c.document()->clearUndoRedoStacks();
  ensureCursorVisible();
}

void Console::display(QString block) {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertText("\n");
  c.insertHtml(block);
  ensureCursorVisible();
}

Console::Console(QWidget * parent): QPlainTextEdit(parent), history(std::vector<QString>()), currHistoryPosition(0), currentLineInsertedInHistory(false), incompleteState(false) {

}
