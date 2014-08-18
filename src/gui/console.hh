#ifndef __SRC_GUI_CONSOLE_H__
#define __SRC_GUI_CONSOLE_H__

#include <QPlainTextEdit>
#include <vector>

class Settings;

class Console: public QPlainTextEdit {
  Q_OBJECT
public:
  Console(QWidget * parent);
  void keyPressEvent(QKeyEvent *e);

public slots:
  
  void incomplete();
  void complete();
  void display(QString msg);

  void visualUpdate(Settings *);

private:

  void updateHistory();
  void rewriteCurrentLine();
  void insertEmptyBlock();

  std::vector<QString> history;
  size_t currHistoryPosition;
  bool currentLineInsertedInHistory;

  bool incompleteState;

signals:
  
  void run(QString line);
  void cancel();

};


#endif //__SRC_GUI_CONSOLE_H__
