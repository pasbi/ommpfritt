#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"
#include "managers/itemmodeladapter.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class StyleListAdapter : public ItemModelAdapter<AbstractStyleListObserver>
{
public:
  using ItemModelAdapter<AbstractStyleListObserver>::ItemModelAdapter;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  friend class AbstractRAIIGuard;
  std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(int row) override;

  // friend class AbstractMoverGuard;
  // std::unique_ptr<AbstractMoverGuard> acquire_mover_guard() override;

  std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(int row) override;
  std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() override;

  Qt::ItemFlags flags(const QModelIndex &index) const;
  Style& item_at(const QModelIndex& index) const override;
};

}  // namespace omm