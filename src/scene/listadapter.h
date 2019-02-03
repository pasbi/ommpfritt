#pragma once

#include <QAbstractItemModel>
#include "scene/itemmodeladapter.h"

class QItemSelection;

namespace omm
{

template<class ItemT>
class ListAdapter : public ItemModelAdapter<List<ItemT>, QAbstractListModel>
{
public:
  explicit ListAdapter(Scene& scene, List<ItemT>& list);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  friend class ::AbstractRAIIGuard;
  std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(int row) override;

  std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const ListMoveContext<ItemT>& context) override;

  std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(int row) override;
  std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  ItemT& item_at(const QModelIndex& index) const override;

  List<ItemT>& list_structure;
};

}  // namespace omm
