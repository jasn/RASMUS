#include <QApplication>
#include <QTableView>
#include <QTextEdit>
#include "ui_main.h"
#include <stdlib/lib.h>
#include <interpreter.hh>
#include <shared/type.hh>
#include <iostream>
#include <relation_model.hh>
#include <map>
#include <QTextCodec>

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  
  Ui::MainWindow ui;
  std::map<QTreeWidgetItem *, QTableView*> tableViews;
  Interpreter *interpreter;

  MainWindow() : tableViews(std::map<QTreeWidgetItem*, QTableView*>()) {
    ui.setupUi(this);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    interpreter = new Interpreter(this);

    QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
    QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
    QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
    QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));

    QObject::connect(ui.environment, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		     this, SLOT(environmentVariableDoubleClicked(QTreeWidgetItem *, int)));

    QObject::connect(interpreter, SIGNAL(updateEnvironment(const char *)), 
		     this, SLOT(environmentChanged(const char *)));
    //QObject::connect(Interpreter, SIGNAL(environmentChange), ui, SLOT(environmentChange());
    ui.console->complete();
  }

public slots:

  void environmentVariableDoubleClicked(QTreeWidgetItem * qtwi, int column) {
    if (qtwi->text(1) == "Rel") {
      if (tableViews.count(qtwi)) {
	tableViews[qtwi]->show();
      } else {
	QTableView *tblView = new QTableView(0);
	tblView->show();
	tblView->setModel(new RelationModel(qtwi->text(0).toStdString().c_str()));
	tableViews[qtwi] = tblView;
      }
    }
  }

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
      // due to strange loading of relations, this is in fact a relation or an error.
      stringRepresentation.append("Rel");
      // update the tableview window.      
    }

    QList<QTreeWidgetItem*> items(ui.environment->findItems(name, Qt::MatchFlag::MatchExactly, 0));
    if (items.size () == 0) {
      QTreeWidgetItem *newItem = new QTreeWidgetItem(ui.environment);
      newItem->setText(0, name);
      newItem->setText(1, stringRepresentation);
    } else {
      QTreeWidgetItem * item = items[0];
      item->setText(1, stringRepresentation);

      if (stringRepresentation == "Rel") {
	if (tableViews.count(item)) {
	  QTableView *tmp = tableViews[item];
	  if (tmp->isVisible()) {
	    tmp->hide();
	  }
	  tableViews.erase(item);
	  delete tmp;
	  environmentVariableDoubleClicked(item, 1);
	}
      }
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
