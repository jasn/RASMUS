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
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
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
#include <QDir>
#include <QMessageBox>
#include "help.hh"
#include <stdlib/gil.hh>

class MainWindow : public QMainWindow {

	Q_OBJECT

public:

	Ui::MainWindow ui;

	Interpreter *interpreter;
	QThread interpreterThread;
	Settings s;

	MainWindow() {
		ui.setupUi(this);
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

		interpreterThread.start();
		interpreter = new Interpreter(nullptr, &s);
		interpreter->moveToThread(&interpreterThread);

		QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
		QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
		QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
		QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));
		QObject::connect(interpreter, SIGNAL(bussy(bool)), ui.console, SLOT(bussy(bool)));

		QObject::connect(this, SIGNAL(doRunContent(QString,QString)), interpreter, SLOT(runContent(QString,QString)));

		QObject::connect(ui.actionCancel, SIGNAL(activated()),
						 interpreter, SLOT(cancel()));

		QObject::connect(ui.console, SIGNAL(cancel()),
						 interpreter, SLOT(cancel()));

		QObject::connect(ui.environment, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
						 this, SLOT(environmentVariableDoubleClicked(QTreeWidgetItem *, int)));

		QObject::connect(interpreter, SIGNAL(updateEnvironment(QString)), 
						 this, SLOT(environmentChanged(QString)));

		QObject::connect(&s, SIGNAL(visualUpdate(Settings *)),
						 ui.console, SLOT(visualUpdate(Settings *)));

		QObject::connect(&s, SIGNAL(visualUpdate(Settings *)),
						 this, SLOT(findRelations(Settings *)));

		QObject::connect(interpreter, SIGNAL(displayRelation(rasmus::stdlib::Relation *)),
						 this, SLOT(displayRelation(rasmus::stdlib::Relation *)));
		
		QObject::connect(this, SIGNAL(unset(QString)),
						 interpreter, SLOT(unset(QString)));

		s.load();		

		ui.console->complete();
	}
	
	~MainWindow() {
		interpreterThread.quit();
		interpreterThread.wait();
	}

public slots:
	void findRelations(Settings * s) {
		QSet<QString> there;
		for (size_t i=0; i < (size_t)ui.environment->topLevelItemCount(); ++i) {
			QTreeWidgetItem * item=ui.environment->topLevelItem(i);
			if (item->data(0, Qt::UserRole) != -1) {
				there.insert(item->text(0));
				continue;
			}
			delete item;
			--i;
		}

		QDir d(s->path());
		for (auto a: d.entryList(QStringList("*.rdb"), QDir::Files | QDir::Readable)) {
			QFileInfo f(a);
			if (there.contains(f.baseName())) continue;
			QTreeWidgetItem *newItem = new QTreeWidgetItem(ui.environment);
			newItem->setText(0, f.baseName());
			newItem->setText(1, "Relation");
			newItem->setData(0, Qt::UserRole, -1);
		}
		ui.environment->sortItems(0, Qt::SortOrder::AscendingOrder);
	}

	void displayRelation(rasmus::stdlib::Relation * r) {
		showTableViewWindow(new RelationModel(r));
		rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
		r->decref();
	}

	void deleteRelation() {

		QString nameOfRelation = QInputDialog::getText(this, "Relation name", "Enter a relation name to delete");

		if (!rasmus::stdlib::gil_execute<bool>([&]{
					return interpreter->relationExists(nameOfRelation);
				})) {

			QString msg = "The relation \"";
			msg.append(nameOfRelation);
			msg.append("\" does not exist in the current environment.");
			// pop up with error 'name does not exist in environment'.
			QMessageBox::warning(this, "Warning", msg);
			return;
		}

		// delete relation.
		QString msg = "This will irretrievably delete the relation \"";
		msg.append(nameOfRelation);
		msg.append("\".");
		QMessageBox::StandardButton btn = QMessageBox::question(this, "Delete relation", msg,
																QMessageBox::Ok | QMessageBox::Cancel);

		if (btn == QMessageBox::Cancel) {
			return;
		}
		
		rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);

		emit unset(nameOfRelation);
		
	}

	void newFile() {
		Editor * e = new Editor(&s);
		QObject::connect(e, SIGNAL(runContent(QString, QString)), interpreter, SLOT(runContent(QString, QString)));
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
		if (!QFile::exists(relPath)) return;

		QString relName = QInputDialog::getText(this, "Relation name", "Enter a name for the relation");

		if (relName == "") return;

		rm_object *rel;

		try {
			rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
			// make sure file exists.

			if (relPath.endsWith("csv", Qt::CaseInsensitive)) {
				rel = rasmus::stdlib::loadRelationFromCSVFile(relPath.toUtf8().data());
			} else {
				rel = rasmus::stdlib::loadRelationFromFile(relPath.toUtf8().data());
			}
		
			interpreter->enterRelationToEnvironment(rel, relName.toUtf8().data());
		} catch (std::exception e) {
			ui.console->complete();
		}
	}

	void environmentVariableDoubleClicked(QTreeWidgetItem * qtwi, int /*column*/) {
		int data = qtwi->data(0, Qt::UserRole).toInt();
		if (data == -1 || data == int(TRel)) {
			showTableViewWindow(new RelationModel(qtwi->text(0).toUtf8().data()));
		}
	}

	void environmentChanged(QString name) {
		std::stringstream repr;
		AnyRet value;		
		QList<QTreeWidgetItem*> items(ui.environment->findItems(name, Qt::MatchFlag::MatchExactly, 0));
		QTreeWidgetItem * item;
		if (items.size() == 0) 
			item = new QTreeWidgetItem(ui.environment);
		else
			item = items[0];

		{
			rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);

			if (!rm_existsGlobalAny(name.toUtf8().data())) {
				delete item;
				return;
			}


			rm_loadGlobalAny(name.toUtf8().data(), &value);

			switch (PlainType(value.type)) {
			case TBool:
				rasmus::stdlib::printBoolToStream(int8_t(value.value), repr);
				break;
			case TInt:
				rasmus::stdlib::printIntToStream(value.value, repr);
				break;
			case TFloat: {
				int64_t cp = value.value;
				rasmus::stdlib::printFloatToStream(*reinterpret_cast<double*>(&cp), repr);
			}
				break;
			case TText: 
				rasmus::stdlib::printTextToStream(reinterpret_cast<rasmus::stdlib::TextBase *>(value.value), repr);
				break;
			case TTup:
				rasmus::stdlib::printTupleToStream(reinterpret_cast<rm_object *>(value.value), repr);
				break;
			case TFunc:
				repr << "Function";
				break;
			case TRel:
				repr << "Relation of " << reinterpret_cast<rasmus::stdlib::Relation *>(value.value)->tuples.size() << " tuples";
				break;
			default:
				repr << "Error";
				break;
			}
		}

		item->setText(0, name.toUtf8().data());
		item->setText(1, QString::fromUtf8(repr.str().c_str()));
		item->setData(0, Qt::UserRole, (int)value.type);
		ui.environment->sortItems(0, Qt::SortOrder::AscendingOrder);
	}

	void showAbout() {
		::showAbout();
	}

	void openFile() {
		QString p = QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("RASMUS Files (*.rm)"));
		if (p.isEmpty()) return;
		
		QFile file(p);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(this, "Error", "Unable to open file " + p + " for reading");
			return;
		}
		
		Editor * e = new Editor(&s, p, QString::fromUtf8(file.readAll()));
		QObject::connect(e, SIGNAL(runContent(QString, QString)), interpreter, SLOT(runContent(QString, QString)));
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
		emit doRunContent(i.fileName(), QString::fromUtf8(file.readAll()));
	}

signals:
	void doRunContent(QString, QString);
	void unset(QString);
};

int main(int argc, char * argv[]) {
  
	QApplication app(argc, argv);

	MainWindow window;

	window.show();
  
	app.exec();
}

#include <main.moc>
