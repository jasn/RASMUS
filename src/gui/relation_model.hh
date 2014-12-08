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
#ifndef RELATION_MODEL_GUARD
#define RELATION_MODEL_GUARD

#include <QAbstractTableModel>
#include <stdlib/relation.hh>
#include <stdlib/lib.h>
#include <stdlib/gil.hh>
#include "ui_relation.h"

class RelationModel : public QAbstractTableModel {
public:
	RelationModel(const char * relationName);
	RelationModel(rasmus::stdlib::Relation *r);

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, 
						Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	std::string relationName;
	rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel;
};

class RelationWindow: public QMainWindow {
	Q_OBJECT
public:
	RelationWindow(RelationModel * model);
public slots:
	void showAbout();
	void saveAsGlobal();
	void save();
	void exportCSV();
	void showPrint();
	void sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

private:
	Ui::RelationWindow ui;
	RelationModel * model;
	bool modified;
};

void showTableViewWindow(RelationModel * rm);

#endif
 
