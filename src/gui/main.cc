// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>

#include "ui_main.h"
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableView>
#include <QTextCodec>
#include <QTextEdit>
#include <QThread>
#include <interpreter.hh>
#include <iostream>
#include <map>
#include <relation_model.hh>

#include <shared/type.hh>
#include <stdlib/lib.h>
#include <stdlib/relation.hh>
#include <settings.hh>
#include <editor.hh>
#include <QThread>
#include <QMessageBox>
#include <ui_about.h>

class MainWindow : public QMainWindow {

	Q_OBJECT

public:
  

	Ui::MainWindow ui;
	std::map<QTreeWidgetItem *, QTableView*> tableViews;
	Interpreter *interpreter;
	QThread interpreterThread;
	Settings s;


	MainWindow() : tableViews(std::map<QTreeWidgetItem*, QTableView*>()) {
		ui.setupUi(this);
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

		interpreterThread.start();
		interpreter = new Interpreter(nullptr);
		interpreter->moveToThread(&interpreterThread);


		QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
		QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
		QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
		QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));

		QObject::connect(ui.actionCancel, SIGNAL(activated()),
						 interpreter, SLOT(cancel()));

		QObject::connect(ui.console, SIGNAL(cancel()),
						 interpreter, SLOT(cancel()));

		QObject::connect(ui.environment, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
						 this, SLOT(environmentVariableDoubleClicked(QTreeWidgetItem *, int)));

		QObject::connect(interpreter, SIGNAL(updateEnvironment(const char *)), 
						 this, SLOT(environmentChanged(const char *)));

		QObject::connect(&s, SIGNAL(visualUpdate(Settings *)),
						 ui.console, SLOT(visualUpdate(Settings *)));

		s.load();

		ui.console->complete();
	}

  ~MainWindow() {
    interpreterThread.quit();
  }

public slots:

	void newFile() {
		Editor * e = new Editor();
		QObject::connect(e, SIGNAL(runContent(QString, QString)), this, SLOT(runContent(QString, QString)));
		e->show();
	}
	
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

	void showAbout() {
		QDialog diag;
		Ui::About about;
		about.setupUi(&diag);
		diag.exec();
	}

	void openFile() {
		QString p = QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("RASMUS Files (*.rm)"));
		if (p.isEmpty()) return;
		
		QFile file(p);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(this, "Error", "Unable to open file " + p + " for reading");
			return;
		}
		
		Editor * e = new Editor(p, QString::fromUtf8(file.readAll()));
		QObject::connect(e, SIGNAL(runContent(QString, QString)), this, SLOT(runContent(QString, QString)));
		e->show();
	}

	void runFile() {
		QString p = QFileDialog::getOpenFileName(this, tr("Run"), QString(), tr("RASMUS Files (*.rm)"));
		if (p.isEmpty()) return;

		QFile file(p);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(this, "Error", "Unable to open file " + p + " for reading");
			return;
		}
		
		QFileInfo i(p);
		
		runContent(i.fileName(), QString::fromUtf8(file.readAll()));
	}

	void runContent(QString name, QString content) {


	}

};

int main(int argc, char * argv[]) {
  
	QApplication app(argc, argv);

	MainWindow window;

	window.show();
  
	app.exec();
}

#include <main.moc>
