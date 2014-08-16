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

RelationModel::RelationModel(const char * relationName) : relationName(relationName) { 
  rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName));
}

RelationModel::~RelationModel() {
  //rm_free(rel);
}

int RelationModel::rowCount(const QModelIndex& parent) const {
  return rel->tuples.size();
}

int RelationModel::columnCount(const QModelIndex& parent) const {
  // rasmus::stdlib::Relation* rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName.c_str()));
  return rel->schema->attributes.size();
}

QVariant RelationModel::data(const QModelIndex& index, int role) const {
  // rasmus::stdlib::Relation* rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName.c_str()));
  std::string val;

  size_t column = index.column();
  size_t row = index.row();
  rasmus::stdlib::AnyValue av = rel->tuples[row]->values[column];
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
      val = rasmus::stdlib::textToString(av.objectValue.getAs<rasmus::stdlib::TextBase>());
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
  rm_sortRel(rel, column, order==Qt::AscendingOrder);
  emit layoutChanged();
}
