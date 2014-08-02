#ifndef __INTERPRETER_GUI_HH_GUARD
#define __INTERPRETER_GUI_HH_GUARD

#include <QObject>
#include <QString>

class InterpreterPrivate;

class Interpreter : public QObject {
  Q_OBJECT
  
public:

  Interpreter(QObject *parent);
  ~Interpreter();

public slots:
  
  void run(QString line);
signals:
  
  void incomplete();
  void complete();
  void display(QString string);
public:
  void doDisplay(QString string);
private:
  InterpreterPrivate *d_ptr;

};


#endif
