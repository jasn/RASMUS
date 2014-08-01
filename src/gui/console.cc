#include <console.hh>

#include <QTextBlock>

void Console::keyPressEvent(QKeyEvent *e) {
    QTextCursor c = textCursor();
    
    int firstInLastBlock = c.document()->lastBlock().position();

    if (c.selectionStart() < firstInLastBlock) {
      c.clearSelection();
      c.movePosition(QTextCursor::End);
      setTextCursor(c);
    }
    switch (e->key()) {
    case Qt::Key_Backspace:
      if (c.positionInBlock()) 
	QPlainTextEdit::keyPressEvent(e);
      break;
    case Qt::Key_Left:
      if (c.positionInBlock())
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
      emit run(c.document()->lastBlock().text());
      break;
      
    default:
      QPlainTextEdit::keyPressEvent(e);
    }
}

void Console::incomplete() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertText("\n");
}

void Console::complete() {
  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::End);
  c.insertBlock();
}
