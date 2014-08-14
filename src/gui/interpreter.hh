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
  void environmentChanged(const char *name);
public:
  void doDisplay(QString string);
  void getCurrentEnvironment();
private:
  InterpreterPrivate *d_ptr;

};


#endif
