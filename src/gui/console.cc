#include <console.hh>
#include <QTextBlock>
#include <iostream>

void Console::keyPressEvent(QKeyEvent *e) {
    QTextCursor c = textCursor();
    
    int firstInLastBlock = c.document()->lastBlock().position() + 4;

    if (c.selectionStart() < firstInLastBlock) {
      c.clearSelection();
      c.movePosition(QTextCursor::End);
      setTextCursor(c);
    }
    switch (e->key()) {
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
    case Qt::Key_Down:
      break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if (c.document()->lastBlock().text().size() <= 4) {
	c.movePosition(QTextCursor::End);
	c.insertText("\n");
      } else
	emit run(c.document()->lastBlock().text().right(c.document()->lastBlock().text().size() -4));
      break;
      
    default:
      QPlainTextEdit::keyPressEvent(e);
    }
}

void Console::incomplete() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertBlock();
  c.insertHtml("<span style=\"color: blue\">...</style>");

  QTextCharFormat cf = currentCharFormat();
  cf.clearForeground();
  setCurrentCharFormat(cf);
  c = textCursor();
  c.insertText(" ");
  ensureCursorVisible();
}

void Console::complete() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertBlock();
  c.insertHtml("<span style=\"color: blue\">&gt;&gt;&gt;</style>");
  QTextCharFormat cf = currentCharFormat();
  cf.clearForeground();
  setCurrentCharFormat(cf);
  c = textCursor();
  c.insertText(" ");
  ensureCursorVisible();
}

void Console::display(QString block) {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertText("\n");
  c.insertHtml(block);
  ensureCursorVisible();
}

Console::Console(QWidget * parent): QPlainTextEdit(parent) {
}
