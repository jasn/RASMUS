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

/**
 * Qt has a framework for displaying tables which have an underlying model
 * for the data. The RelationModel class is that model.
 *
 * The RelationModel is responsible for retrieving data from the interpreter
 * in order to display RASMUS relations in tables.
 */
class RelationModel : public QAbstractTableModel {
public:
	/**
	 * Constructor for RelationModel.
	 *
	 * @param relationName is the variable name of a relation we want a model of.
	 */
	RelationModel(const char * relationName);

	/**
	 * Constructor for RelationModel.
	 *
	 * @param r is a pointer to a rasmus::stdlib::Relation we want a model of.
	 */
	RelationModel(rasmus::stdlib::Relation *r);

	/**
	 * @override
	 * @return the number of rows in the data, i.e. the number of tuples in
	 * the relation.
	 */
	int rowCount(const QModelIndex& parent) const override;

	/**
	 * @override
	 * @return the number of columns in the data, i.e. the number of attributes
	 * in the tuples of the relation.
	 */
	int columnCount(const QModelIndex& parent) const override;

	/**
	 * The method returns data for a given index dependent on the 'role'.
	 * The role is a way to query the model for various parameters of a cell
	 * E.g. the width/height of a cell, or its contents.
	 * @param index an entry of the table.
	 * @param role is an enum of Qt::ItemDataRole
	 * @override
	 */
	QVariant data(const QModelIndex& index, int role) const override;

	/**
	 * Similar to data(QModelIndex&, int), but returns data for the
	 * header of the table.
	 */
	QVariant headerData(int section, 
						Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override;

	/**
	 * flags of a cell tells Qt whether the cell is selectable/editable etc.
	 * @param index is the cell the query is asked on.
	 */
	Qt::ItemFlags flags(const QModelIndex &index) const overide;

	/**
	 * Updates the contents of the cell index with the data value.
	 */
	bool setData(const QModelIndex &index,
				 const QVariant &value,
				 int role = Qt::EditRole) override;

	/**
	 * The user can click on a column to sort by that column.
	 * In order to sort the Qt framework asks the model the sort the data
	 * in either ascending or descending order.
	 * @param column the attribute to sort by
	 * @param order is either Qt::AscendingOrder og Qt::DescendingOrder
	 */
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	std::string relationName;
	rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel;
};

/**
 * This is the window that displays a relation, based on a RelationModel.
 * See relation.ui for how it looks.
 */
class RelationWindow: public QMainWindow {
	Q_OBJECT
public:
	/**
	 * The constructor sets up the Window. Since it displays a relation
	 * it needs to have a model of the data it is displaying.
	 * @param model the model of the data to be displayed. See RelationModel for
	 * more information.
	 */
	RelationWindow(RelationModel * model);
public slots:
	/**
	 * When clicking 'Help->About' in the menu this slot receives a signal
	 * and displays the 'About' dialog. See about.ui for how it looks.
	 */
	void showAbout();

	/**
	 * The table being displayed might be a result of a computation that did not
	 * assign the result to any variable. Later one might want to save the
	 * relation anyway, so via 'File->Save As Global' a prompt is opened
	 * that asks for the desired variable name to associate with this relation.
	 * This slot is invoked when the user click 'File->Save As Global' or when
	 * clicking 'File->Save' and the relation is currently not associated with
	 * a variable name.
	 */
	void saveAsGlobal();

	/**
	 * Similar to saveAsGlobal() except if the relation already has a variable
	 * name, that variable is overwritten with the current relation.
	 */
	void save();

	/**
	 * It is useful to be able to export data after having worked with it in
	 * RASMUS. This method is invoked when the user clicks 'File->Export CSV'.
	 * The slot opens a dialog for where to put the csv file (and what to name
	 * it).
	 */
	void exportCSV();

	/**
	 * Sometimes one wants to print a relation.
	 * This opens the print dialog and sends a print request to a printer.
	 */
	void showPrint();

	/**
	 * It is possible to reorder the columns of a table. This method is invoked
	 * whenever the user changes the order of the columns. The user changes the
	 * layout by dragging one column left/right.
	 * @param logicalIndex is not used.
	 * @param oldVisualIndex the old index of where the moved column was
	 * @param newVisualIndex the new position of the moved column.
	 */
	void sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

private:
	Ui::RelationWindow ui;
	RelationModel * model;
	bool modified;
};

/**
 * Opens a TableViewWindow for the RelationModel *rm.
 */
void showTableViewWindow(RelationModel * rm);

#endif
 
