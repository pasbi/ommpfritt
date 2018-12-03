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

  void beginInsertStyles(int row) override;
  void endInsertStyles() override;
  void beginResetStyles() override;
  void endResetStyles() override;

  Qt::ItemFlags flags(const QModelIndex &index) const;


private:
  Scene& m_scene;
};

}  // namespace omm