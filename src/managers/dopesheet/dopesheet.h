#pragma once

#include <QAbstractItemModel>

namespace omm
{

class Scene;
class Track;

class AbstractPropertyOwner;

class DopeSheet : public QAbstractItemModel
{
public:
  explicit DopeSheet(Scene& scene);

  enum class IndexType { Track, Owner, None };

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QModelIndex index(Track* track) const;
  QModelIndex index(AbstractPropertyOwner* owner) const;
  IndexType index_type(const QModelIndex& index) const;
  Track* track(const QModelIndex& index) const;
  AbstractPropertyOwner* owner(const QModelIndex& index) const;

protected:
  int level(const QModelIndex& index);

private:
  Scene& m_scene;

};

}  // namespace omm
