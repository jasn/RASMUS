#include "relation_model.hh"
#include <QString>
#include <sstream>
#include <iostream>
#include <shared/type.hh>
#include <QSize>
#include <stdlib/anyvalue.hh>

RelationModel::RelationModel(const char * relationName) : relationName(relationName) { 
  rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName));
}

RelationModel::~RelationModel() {}

int RelationModel::rowCount(const QModelIndex& parent) const {
  return rel->tuples.size();
}

int RelationModel::columnCount(const QModelIndex& parent) const {
  // rasmus::stdlib::Relation* rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName.c_str()));
  return rel->schema->attributes.size();
}

QVariant RelationModel::data(const QModelIndex& index, int role) const {
  // rasmus::stdlib::Relation* rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName.c_str()));
  if (role == Qt::DisplayRole) {
    size_t column = index.column();
    size_t row = index.row();
    rasmus::stdlib::AnyValue av = rel->tuples[row]->values[column];
    switch (av.type) {
    case TInt:
      return QVariant("int");
      break;
    case TBool:
      return QVariant("bool");
      break;
    case TText:
      //return QVariant(QString::fromStdString(textToString(av.objectValue)));
      return QVariant("text");
    default:
      return QVariant("something else");
      break;
      
    }
    return QVariant("hmm");
  }

  return QString("");
}

QVariant RelationModel::headerData(int section, Qt::Orientation orientation, int role) const {

  if (orientation != Qt::Horizontal) return QVariant("");
  if (role == Qt::SizeHintRole) {
    return QVariant(QSize(50,20));
  }

  // rasmus::stdlib::Relation* rel = static_cast<rasmus::stdlib::Relation*>(rm_loadRel(relationName.c_str()));

  if (section < rel->schema->attributes.size()) {
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

    return QVariant(QString::fromStdString(ss.str()));
  } else {
    return QVariant("foo");
  }


}
