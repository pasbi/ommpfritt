#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"

class QItemSelection;

namespace omm
{

class StylePool;
class Object;

class StyleListAdapter : public QAbstractListModel, public AbstractStyleListObserver
{
  Q_OBJECT

public:
  explicit StyleListAdapter(StylePool& style_pool);
  ~StyleListAdapter();

  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  StylePool& style_pool() const;

  void beginInsertObject(int row) override;
  void endInsertObject() override;

private:
  StylePool& m_style_pool;
};

}  // namespace omm