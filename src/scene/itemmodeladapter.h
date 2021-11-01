#pragma once

#include "objects/object.h"
#include <QAbstractItemModel>

namespace omm
{
class Scene;

template<typename StructureT, typename ItemT, typename ItemModel>
class ItemModelAdapter : public ItemModel
{
  static_assert(std::is_base_of<QAbstractItemModel, ItemModel>::value,
                "ItemModel must be derived from QAbstractItemModel");

public:
  using structure_type = StructureT;
  explicit ItemModelAdapter(Scene& scene, StructureT& structure);
  virtual ~ItemModelAdapter() = default;
  ItemModelAdapter(const ItemModelAdapter&) = delete;
  ItemModelAdapter(ItemModelAdapter&&) = delete;
  ItemModelAdapter& operator=(const ItemModelAdapter&) = delete;
  ItemModelAdapter& operator=(ItemModelAdapter&&) = delete;

  [[nodiscard]] Qt::DropActions supportedDragActions() const override;
  [[nodiscard]] Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData(const QMimeData* data,
                       Qt::DropAction action,
                       int row,
                       int column,
                       const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data,
                    Qt::DropAction action,
                    int row,
                    int column,
                    const QModelIndex& parent) override;
  [[nodiscard]] QStringList mimeTypes() const override;
  [[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
  virtual ItemT& item_at(const QModelIndex& index) const = 0;
  virtual QModelIndex index_of(ItemT& item) const = 0;
  Scene& scene;
  StructureT& structure;
};

}  // namespace omm
