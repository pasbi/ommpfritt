#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"

namespace omm
{

class Scene;

class Object;


class ObjectTreeAdapter : public QAbstractItemModel, public AbstractObjectTreeAdapter
{
  Q_OBJECT

public:
  using RootObject = Object;
  explicit ObjectTreeAdapter(Object& root_object);
  ~ObjectTreeAdapter();

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Object& object_at(const QModelIndex& index) const;

  QModelIndex index_of(Object& object) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  void beginInsertObjects(Object& parent, int start, int end) override;
  void endInsertObjects() override;

private:
  Object& m_root;

};

}  // namespace omm