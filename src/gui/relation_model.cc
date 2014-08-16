#include "relation_model.hh"
#include <QString>
#include <sstream>
#include <iostream>
#include <shared/type.hh>
#include <QSize>
#include <stdlib/anyvalue.hh>
#include <string>
#include <stdlib/text.hh>

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
      return QVariant(static_cast<int>(av.intValue));
      break;
    case TBool:
      switch (av.boolValue) {
      case RM_TRUE:
	return QVariant("true");
      case RM_FALSE:
	return QVariant("false");
      case RM_NULLBOOL:
	return QVariant("?-Bool");
      default:
	return QVariant("Unkown (internal error)");
      }
      break;
    case TText:
      {
	std::string str = 
	  rasmus::stdlib::textToString(av.objectValue.getAs<rasmus::stdlib::TextBase>());
	return QVariant(str.c_str());
      }
    default:
      return QVariant("something else");
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
