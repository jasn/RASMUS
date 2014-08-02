#ifndef __SRC_GUI_CONSOLE_H__
#define __SRC_GUI_CONSOLE_H__

#include <QPlainTextEdit>

class Console: public QPlainTextEdit {
  Q_OBJECT
public:
  Console(QWidget * parent);
  void keyPressEvent(QKeyEvent *e);

public slots:
  
  void incomplete();
  void complete();
  void display(QString msg);
signals:
  
  void run(QString line);

};


#endif //__SRC_GUI_CONSOLE_H__
