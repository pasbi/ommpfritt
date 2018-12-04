#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class ObjectTreeAdapter : public QAbstractItemModel, public AbstractObjectTreeObserver
{
  Q_OBJECT

public:
  explicit ObjectTreeAdapter(Scene& scene);
  ~ObjectTreeAdapter();

  using item_type = Object;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::DropActions supportedDragActions() const override;
  Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData( const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent ) const override;
  bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                     int row, int column, const QModelIndex &parent ) override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList &indexes) const override;

  Object& object_at(const QModelIndex& index) const;

  QModelIndex index_of(Object& object) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  friend class AbstractInserterGuard;
  std::unique_ptr<AbstractInserterGuard>
  acquire_inserter_guard(Object& parent, int row) override;

  friend class AbstractMoverGuard;
  std::unique_ptr<AbstractMoverGuard>
  acquire_mover_guard(const MoveObjectTreeContext& context) override;

  friend class AbstractRemoverGuard;
  std::unique_ptr<AbstractRemoverGuard> acquire_remover_guard(const Object& object) override;

  friend class AbstractReseterGuard;
  std::unique_ptr<AbstractReseterGuard> acquire_reseter_guard() override;

  Scene& scene() const;

private:
  Scene& m_scene;

  std::vector<omm::MoveObjectTreeContext>
  make_new_contextes(const QMimeData* data, int row, const QModelIndex& parent) const;
  bool m_last_move_was_noop;
};

}  // namespace omm