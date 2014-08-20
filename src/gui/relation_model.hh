#ifndef RELATION_MODEL_GUARD
#define RELATION_MODEL_GUARD

#include <QAbstractTableModel>
#include <stdlib/relation.hh>
#include <stdlib/lib.h>



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

private:
  std::string relationName;
  rasmus::stdlib::RefPtr<rasmus::stdlib::Relation> rel;
};

void showTableViewWindow(RelationModel * rm);

#endif
