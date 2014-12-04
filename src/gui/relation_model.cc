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
#include <stdlib/relation.hh>
#include <string>
#include <stdlib/text.hh>
#include <stdlib/lib.h>
#include <QTableView>
#include <QHeaderView>
#include "help.hh"
#include <QFileDialog>
#include <QInputDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPainter>
#include <numeric>

namespace rs = rasmus::stdlib;

namespace {

std::string getHeaderText(const rs::Attribute &a) {

	std::stringstream ss;
	ss << a.name;
	ss << " : ";

	ss << a.type;

	return ss.str();

}

std::string printHelper(size_t row, size_t column, rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel) {

	std::stringstream ss;
	std::vector<size_t> &pi = rel->permutation;
	rs::AnyValue av = rel->tuples[row]->values[pi[column]];
	switch (av.type) {
	case TInt:
		rs::printIntToStream(av.intValue, ss);
		break;
	case TFloat:
		rs::printFloatToStream(av.floatValue, ss);
		break;
	case TBool:
		rs::printBoolToStream(av.boolValue, ss);
		break;
	case TText: {
		rs::printTextToStream(av.objectValue.getAs<rs::TextBase>(), ss);
		break;
	}
	default:
		ss << "Something else";
	}

	return ss.str();

}

}


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
	switch (role) {
	case Qt::DisplayRole: {
		rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
		return QVariant(QString::fromStdString(::printHelper(index.row(), index.column(), this->rel)));
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
	std::vector<size_t> &pi = rel->permutation;
	std::string s = ::getHeaderText(rel->schema->attributes[pi[section]]);

	switch (role) {
	case Qt::DisplayRole:
		return QVariant(QString::fromStdString(s));
	case Qt::SizeHintRole:
		return QVariant(QSize(s.length()*5, 25));
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

RelationWindow::RelationWindow(RelationModel * model): model(model), modified(false) {
	ui.setupUi(this);
	model->setParent(this);
	ui.view->setModel(model);
	setAttribute(Qt::WA_DeleteOnClose);
	if (!model->relationName.empty())
		setWindowTitle(QString::fromUtf8(model->relationName.c_str()) + " - Relation");

	// Permutation of attributes
	// Also need to create a slot for the signal QHeaderView::sectionMoved
	ui.view->horizontalHeader()->setMovable(true);
	
	QObject::connect(ui.view->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), 
					 this, SLOT(sectionMoved(int, int, int)));
}

void RelationWindow::sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex) {

	// if unmodified, make a copy
	if (!this->modified) {
		// copy
		this->model->rel = rs::copyRelation(this->model->rel);
		// make a STAR appear in the window.
		setWindowTitle(windowTitle() + "*");
		this->modified = true;
	}
	static bool inSectionMoved=false;
	if (inSectionMoved) return;
	inSectionMoved = true;
	struct _ {~_() {inSectionMoved=false;} } _;
	

	ui.view->horizontalHeader()->moveSection(newVisualIndex, oldVisualIndex);
	
	rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel = this->model->rel;

	std::vector<size_t> &pi = rel->permutation;
	size_t x = pi[oldVisualIndex];
	pi.erase(pi.begin()+oldVisualIndex);
	pi.insert(pi.begin()+newVisualIndex, x);
	
	// save permutation to file.
	//emit permutationChanged(this->model);
}

void RelationWindow::showAbout() {
	::showAbout();
}

void RelationWindow::saveAsGlobal() {
	QString relName = QInputDialog::getText(this, tr("Relation name"), tr("Enter a name for the relation"));
	if (relName.isEmpty()) return;
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	rm_saveGlobalAny(relName.toUtf8().data(), reinterpret_cast<uint64_t>(model->rel.get()), TRel);
	setWindowTitle(relName + " - Relation");
}

void RelationWindow::exportCSV() {
	QString name="relation.csv";
	if (!model->relationName.empty()) 
		name = QString::fromUtf8(model->relationName.c_str())+".csv";
	QString p = QFileDialog::getSaveFileName(this, tr("Export as"), name, tr("CSV Files (*.csv)"));
	if (p.isEmpty()) return;
	rasmus::stdlib::gil_lock_t lock(rasmus::stdlib::gil);
	rasmus::stdlib::saveCSVRelationToFile(model->rel.get(), p.toUtf8().data());
}

void RelationWindow::showPrint() {
	
	QPrinter *printer = new QPrinter(QPrinter::HighResolution);
	QPrintDialog *pdlg = new QPrintDialog(printer, this);
	pdlg->setWindowTitle(tr("Print Relation"));
	
	//Tell the printer object to print the file <out>

	if (!pdlg->exec() == QDialog::Accepted) {
		return;
	}
	
	QPainter p;
	p.begin(printer);
	rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel = model->rel;
	std::vector<double> xs(rel->schema->attributes.size(), 1e13);
	std::vector<double> ys(xs.size(), 0);
	
	for (size_t i = 0; i < xs.size(); ++i) {
		std::string tmp = ::getHeaderText(rel->schema->attributes[i]);
		xs[i] = p.fontMetrics().width(QString::fromStdString(tmp));
	}

	for (size_t j = 0; j < rel->tuples.size(); ++j) {
		for (size_t i = 0; i < xs.size(); ++i) {
			std::string tmp = ::printHelper(j, i, rel);
			xs[i] = std::max<double>(xs[i],
							 p.fontMetrics().width(
								 QString::fromStdString(tmp)));
							 
		}
	}

	const double spacing = 60;

	double xmin = 0.0;
	double xmax = 1e13;
	while (xmax - xmin >= 1) {

		double x = (xmax + xmin)/2;
		for (size_t i = 0; i < ys.size(); ++i) {
			ys[i] = std::min<double>(x, xs[i]);
			
		}

		if (std::accumulate(ys.begin(), ys.end(), 0.0) + spacing * (ys.size()-1) > 
			p.viewport().width()) {
			
			xmax = x;

		} else {
			xmin = x;
		}
		
	}

	bool shouldPrintHeader = true;
	double whereToStartY = 0.0;
	size_t currentRow = 0;
	while (true) {
		if (shouldPrintHeader) {
			double whereToStartX = 0.0;
			double headerHeight = 0.0;
			for (size_t i = 0; i < xs.size(); ++i) {
				std::string tmp = ::getHeaderText(rel->schema->attributes[i]);
				QRectF allowed(whereToStartX, whereToStartY, ys[i], 1e13);
				QRectF boundingRect;
				p.drawText(allowed,
						   Qt::TextWordWrap | Qt::AlignTop, 
						   QString::fromStdString(tmp),
						   &boundingRect);
				
				whereToStartX += ys[i] + spacing;

				headerHeight = std::max<double>(headerHeight, boundingRect.height());

				if (i+1 == xs.size()) continue;
				auto x = p.pen();
				p.setPen(10);
				p.drawLine(whereToStartX - spacing/2, 0, whereToStartX - spacing/2, p.viewport().height());
				p.setPen(x);
			}
			auto x = p.pen();
			p.setPen(20);
			p.drawLine(0, headerHeight + 20, p.viewport().width(), headerHeight+20);
			p.setPen(x);

			whereToStartY += headerHeight + 40;
			shouldPrintHeader = false;
		}

		if (currentRow == rel->tuples.size()) break;

		double rowHeight = 0.0;
		double whereToStartX = 0.0;
		for (size_t i = 0; i < xs.size(); ++i) {
			std::string tmp = ::printHelper(currentRow, i, rel);

			QRectF allowed(whereToStartX, whereToStartY, ys[i], 1e13);
			QRectF boundingRect;
			p.drawText(allowed,
					   Qt::TextWordWrap | Qt::AlignTop, 
					   QString::fromStdString(tmp),
					   &boundingRect);
				
			whereToStartX += ys[i] + spacing;

			rowHeight = std::max<double>(rowHeight, boundingRect.height());

		}

		whereToStartY += rowHeight + 10;

		if (whereToStartY + 50 > p.viewport().height()) {
			printer->newPage();
			shouldPrintHeader = true;
			whereToStartY = 0.0;
		}

		++currentRow;
	}

	p.end();
	
}

RelationWindow * showTableViewWindow(RelationModel * rm) {
	RelationWindow * w = new RelationWindow(rm);
	w->show();
	return w;
}
