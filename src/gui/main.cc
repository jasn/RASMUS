#include "ui_main.h"
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableView>
#include <QTextCodec>
#include <QTextEdit>

#include <interpreter.hh>
#include <iostream>
#include <map>
#include <relation_model.hh>

#include <shared/type.hh>
#include <stdlib/lib.h>
#include <stdlib/relation.hh>
#include <settings.hh>

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  
  Ui::MainWindow ui;
  std::map<QTreeWidgetItem *, QTableView*> tableViews;
  Interpreter *interpreter;
  Settings s;

  MainWindow() : tableViews(std::map<QTreeWidgetItem*, QTableView*>()) {
    ui.setupUi(this);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    interpreter = new Interpreter(this);

    QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
    QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
    QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
    QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));

    QObject::connect(ui.console, SIGNAL(cancel()),
		     interpreter, SLOT(cancel()));

    QObject::connect(ui.environment, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		     this, SLOT(environmentVariableDoubleClicked(QTreeWidgetItem *, int)));

    QObject::connect(interpreter, SIGNAL(updateEnvironment(const char *)), 
		     this, SLOT(environmentChanged(const char *)));

    ui.console->complete();
  }

public slots:
  void showPreferences() {
    s.show();
  }

  void importRelation() {
    QString relPath = QFileDialog::getOpenFileName(this, tr("Open File"),
						   QString(), 
						   tr("Rasmus Files (*.rdb *.csv)"));
    if (relPath == "") return;

    QString relName = QInputDialog::getText(this, "Relation name", "Enter a name for the relation");

    if (relName == "") return;

    rm_object *rel;

    if (relPath.endsWith("csv", Qt::CaseInsensitive)) {
      rel = rasmus::stdlib::loadRelationFromCSVFile(relPath.toStdString().c_str());
    } else {
      rel = rasmus::stdlib::loadRelationFromFile(relPath.toStdString().c_str());
    }

    interpreter->enterRelationToEnvironment(rel, relName.toStdString().c_str());

  }

  void environmentVariableDoubleClicked(QTreeWidgetItem * qtwi, int column) {
    if (qtwi->text(1) == "Rel") {
      if (tableViews.count(qtwi)) {
	tableViews[qtwi]->show();
      } else {
	QTableView *tblView = new QTableView(0);
	tblView->setSortingEnabled(true);
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
