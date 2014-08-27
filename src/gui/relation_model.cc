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
#include "relation_model.hh"
#include <QString>
#include <sstream>
#include <iostream>
#include <shared/type.hh>
#include <QSize>
#include <stdlib/anyvalue.hh>
#include <string>
#include <stdlib/text.hh>
#include <stdlib/lib.h>
#include <QTableView>
#include <QHeaderView>
#include "help.hh"
#include <QFileDialog>
#include <QInputDialog>

namespace rs = rasmus::stdlib;

RelationModel::RelationModel(const char * relationName) : relationName(relationName) { 
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	AnyRet rv;
	rm_loadGlobalAny(relationName, &rv);
	rel = rs::RefPtr<rs::Relation>(reinterpret_cast<rs::Relation*>(rv.value));
}

RelationModel::RelationModel(rasmus::stdlib::Relation *r) {
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	rel = rasmus::stdlib::RefPtr<rasmus::stdlib::Relation>(r);
}

int RelationModel::rowCount(const QModelIndex&) const {
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	return rel->tuples.size();
}

int RelationModel::columnCount(const QModelIndex&) const {
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	return rel->schema->attributes.size();
}

QVariant RelationModel::data(const QModelIndex& index, int role) const {
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	std::string val;

	size_t column = index.column();
	size_t row = index.row();
	rs::AnyValue av = rel->tuples[row]->values[column];
	switch (av.type) {
	case TInt: {
		std::stringstream tmpss;
		tmpss << av.intValue;
		val = tmpss.str();
		break;
	}
	case TBool:
		switch (av.boolValue) {
		case RM_TRUE:
			val = "true";
			break;
		case RM_FALSE:
			val = "false";
			break;
		case RM_NULLBOOL:
			val = "?-Bool";
			break;
		default:
			val = "Unknown (internal error)";
		}
		break;
	case TText: {
		val = rs::textToString(av.objectValue.getAs<rs::TextBase>());
		break;
    }
	default:
		val = "Something else";
	}


	switch (role) {
	case Qt::DisplayRole: {
		return QVariant(QString::fromStdString(val));
	}
	case Qt::SizeHintRole: {
		return QSize(100,30);
		break;
	}
	default:
		return QVariant();
	}

}

QVariant RelationModel::headerData(int section, Qt::Orientation orientation, int role) const {
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	if (orientation != Qt::Horizontal) return QVariant("f");

	std::stringstream ss;
	ss << rel->schema->attributes[section].name;
	ss << " : ";
	switch (rel->schema->attributes[section].type) {
	case TText:
		ss << "Text";
		break;
	case TInt:
		ss << "Int";
		break;
	default:
		ss << "Unknown";
		break;
	}

	switch (role) {
	case Qt::DisplayRole:
		return QVariant(QString::fromStdString(ss.str()));
	case Qt::SizeHintRole:
		return QVariant(QSize(ss.str().length()*5, 25));
	default:
		return QVariant("");
	}

}

void RelationModel::sort(int column, Qt::SortOrder order) {
	{
		rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
		rm_sortRel(rel.get(), column, order==Qt::AscendingOrder);
	}
	emit layoutChanged();
}

RelationWindow::RelationWindow(RelationModel * model): model(model) {
	ui.setupUi(this);
	model->setParent(this);
	ui.view->setModel(model);
	setAttribute(Qt::WA_DeleteOnClose);
	if (!model->relationName.empty())
		setWindowTitle(QString::fromUtf8(model->relationName.c_str()) + " - Relation");
}


void RelationWindow::showAbout() {
	::showAbout();
}

void RelationWindow::saveAsGlobal() {
	QString relName = QInputDialog::getText(this, tr("Relation name"), tr("Enter a name for the relation"));
	if (relName.isEmpty()) return;
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	rm_saveGlobalAny(relName.toUtf8().data(), reinterpret_cast<uint64_t>(model->rel.get()), TRel);
}

void RelationWindow::exportCSV() {
	QString name="relation.csv";
	if (!model->relationName.empty()) 
		name = QString::fromUtf8(model->relationName.c_str())+".csv";
	QString p = QFileDialog::getSaveFileName(this, tr("Export as"), name, tr("CSV Files (*.csv)"));
	if (p.isEmpty()) return;
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	//rasmus::stdlib::saveCSVRelationToFile(model->rel.get(), name.toUtf8().data());
	rasmus::stdlib::saveCSVRelationToFile(model->rel.get(), p.toUtf8().data());
}

void showTableViewWindow(RelationModel * rm) {
	RelationWindow * w = new RelationWindow(rm);
	w->show();
}
