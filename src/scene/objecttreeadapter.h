#pragma once

#include <QAbstractItemModel>
#include "scene/itemmodeladapter.h"
#include "scene/tree.h"
#include "scene/abstractstructureobserver.h"
#include "objects/object.h"
#include "abstractraiiguard.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class ObjectTreeAdapter : public ItemModelAdapter<Tree<Object>, QAbstractItemModel>
{
public:
  explicit ObjectTreeAdapter(Scene& scene, Tree<Object>& tree);
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Object& item_at(const QModelIndex& index) const override;
  QModelIndex index_of(Object &object) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent ) override;

  bool canDropMimeData( const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent ) const override;

  friend class ::AbstractRAIIGuard;
  std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(Object& parent, int row) override;

  std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const ObjectTreeMoveContext& context) override;

  std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(const Object& object) override;
  std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() override;

  constexpr static int TAGS_COLUMN = 2;
  constexpr static int VISIBILITY_COLUMN = 1;
  constexpr static int OBJECT_COLUMN = 0;

  std::size_t max_number_of_tags_on_object() const;

public:
  Tag* current_tag_predecessor = nullptr;
  Tag* current_tag = nullptr;

};

}  // namespace omm
