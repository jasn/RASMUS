#ifndef __INTERPRETER_GUI_HH_GUARD
#define __INTERPRETER_GUI_HH_GUARD

#include <QObject>
#include <QString>
#include <stdlib/lib.h>

class InterpreterPrivate;

class Interpreter : public QObject {
  Q_OBJECT
  
public:
  
  Interpreter(QObject *parent);
  ~Interpreter();
  void environmentChanged(const char *name);
  void enterRelationToEnvironment(rm_object * rel, const char * name);

public slots:
  
  void run(QString line);
  void cancel();

signals:
  
  void incomplete();
  void complete();
  void display(QString string);
  void updateEnvironment(const char * name);

public:
  void doDisplay(QString string);
  void getCurrentEnvironment();
private:
  InterpreterPrivate *d_ptr;

};


#endif
