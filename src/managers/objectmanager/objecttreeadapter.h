#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"
#include "managers/itemmodeladapter.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class ObjectTreeAdapter
  : public ItemModelAdapter<AbstractObjectTreeObserver>
{
  Q_OBJECT  // TODO remove

public:
  using ItemModelAdapter<AbstractObjectTreeObserver>::ItemModelAdapter;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Object& item_at(const QModelIndex& index) const override;
  QModelIndex index_of(Object& object) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  friend class AbstractRAIIGuard;
  std::unique_ptr<AbstractRAIIGuard>
  acquire_inserter_guard(Object& parent, int row) override;

  std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const ObjectTreeMoveContext& context) override;

  std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(const Object& object) override;
  std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() override;

};

}  // namespace omm