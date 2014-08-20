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

namespace rs = rasmus::stdlib;

RelationModel::RelationModel(const char * relationName) : relationName(relationName) { 
  AnyRet rv;
  rm_loadGlobalAny(relationName, &rv);
  rel = rs::RefPtr<rs::Relation>(reinterpret_cast<rs::Relation*>(rv.value));
  
}

RelationModel::RelationModel(rasmus::stdlib::Relation *r) : rel(r) { }

int RelationModel::rowCount(const QModelIndex& parent) const {
  return rel->tuples.size();
}

int RelationModel::columnCount(const QModelIndex& parent) const {
  // rs::Relation* rel = static_cast<rs::Relation*>(rm_loadRel(relationName.c_str()));
  return rel->schema->attributes.size();
}

QVariant RelationModel::data(const QModelIndex& index, int role) const {
  // rs::Relation* rel = static_cast<rs::Relation*>(rm_loadRel(relationName.c_str()));
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
  rm_sortRel(rel.get(), column, order==Qt::AscendingOrder);
  emit layoutChanged();
}

void showTableViewWindow(RelationModel * rm) {
  QTableView *tblView = new QTableView(0);
  tblView->setSortingEnabled(true);
  tblView->show();

  tblView->setAttribute(Qt::WA_DeleteOnClose);

  tblView->setModel(rm);

  rm->setParent(tblView);

}
