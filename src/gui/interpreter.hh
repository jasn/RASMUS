#ifndef __INTERPRETER_GUI_HH_GUARD
#define __INTERPRETER_GUI_HH_GUARD

#include <QObject>
#include <QString>
#include <stdlib/lib.h>

namespace rasmus {
namespace stdlib {
  class Relation;
}
}

class InterpreterPrivate;

class Interpreter : public QObject {
  Q_OBJECT
  
public:
  
  Interpreter(QObject *parent);
  ~Interpreter();
  void environmentChanged(const char *name);
  void enterRelationToEnvironment(rm_object * rel, const char * name);
  void doDisplayRelation(rasmus::stdlib::Relation *);

public slots:
  
  void run(QString line);
  void cancel();
  void runContent(QString name, QString content);

signals:
  void incomplete();
  void complete();
  void display(QString string);
  void updateEnvironment(const char * name);
  void bussy(bool);

  void displayRelation(rasmus::stdlib::Relation *);

public:
  void doDisplay(QString string);
  void getCurrentEnvironment();
private:
  InterpreterPrivate *d_ptr;

};


#endif
