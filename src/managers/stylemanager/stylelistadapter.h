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
  explicit StyleListAdapter(Scene& scene);
  ~StyleListAdapter();

  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Scene& scene() const;

  void beginInsertObject(int row) override;
  void endInsertObject() override;

private:
  Scene& m_scene;
};

}  // namespace omm