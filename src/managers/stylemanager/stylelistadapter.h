#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class StyleListAdapter : public QAbstractListModel, public AbstractStyleListObserver
{
  Q_OBJECT

public:
  using item_type = Style;

  explicit StyleListAdapter(Scene& scene);
  ~StyleListAdapter();

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Scene& scene() const;

  friend class AbstractInserterGuard;
  std::unique_ptr<AbstractInserterGuard> acquire_inserter_guard(int row) override;

  // friend class AbstractMoverGuard;
  // std::unique_ptr<AbstractMoverGuard> acquire_mover_guard() override;

  friend class AbstractRemoverGuard;
  std::unique_ptr<AbstractRemoverGuard> acquire_remover_guard(int row) override;

  friend class AbstractReseterGuard;
  std::unique_ptr<AbstractReseterGuard> acquire_reseter_guard() override;

  Qt::ItemFlags flags(const QModelIndex &index) const;


private:
  Scene& m_scene;
};

}  // namespace omm