#include <QApplication>
#include <QTextEdit>
#include "ui_main.h"
#include <stdlib/lib.h>
#include <interpreter.hh>
#include <shared/type.hh>
#include <iostream>

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  
  Ui::MainWindow ui;

  Interpreter *interpreter;

  MainWindow() {
    ui.setupUi(this);
    interpreter = new Interpreter(this);
    
    QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
    QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
    QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
    QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));
    QObject::connect(interpreter, SIGNAL(updateEnvironment(const char *)), this, SLOT(environmentChanged(const char *)));
    //QObject::connect(Interpreter, SIGNAL(environmentChange), ui, SLOT(environmentChange());
    ui.console->complete();
  }

public slots:

  void environmentChanged(const char *name) {
    // do type look up
    AnyRet *valuePtr = new AnyRet();
    rm_loadGlobalAny(name, valuePtr);
    QString stringRepresentation;
    switch (Type(valuePtr->type)) {
    case TBool:
      stringRepresentation.append("Bool");
      break;
    case TInt:
      stringRepresentation.append("Int");
      break;
    case TText: //It's a Text
      stringRepresentation.append("Text");
      break;
    case TTup:
      stringRepresentation.append("Tup");
      break;
    case TFunc:
      stringRepresentation.append("Func");
      break;
    default:
      stringRepresentation.append("Unknown");
    }

    QList<QTreeWidgetItem*> items(ui.environment->findItems(name, Qt::MatchFlag::MatchExactly, 0));
    if (items.size () == 0) {
      QTreeWidgetItem *newItem = new QTreeWidgetItem(ui.environment);
      newItem->setText(0, name);
      newItem->setText(1, stringRepresentation);
    } else {
      items[0]->setText(1, stringRepresentation);
    }

    ui.environment->sortItems(0, Qt::SortOrder::AscendingOrder);

  }

};

int main(int argc, char * argv[]) {
  
  QApplication app(argc, argv);

  MainWindow window;

  window.show();
  
  app.exec();
  
    
}

#include <main.moc>
