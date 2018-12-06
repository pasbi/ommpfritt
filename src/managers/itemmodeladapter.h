#pragma once

#include <QAbstractListModel>

namespace omm
{

class Scene;

template<typename T, typename ObserverT>
class ItemModelAdapter : public QAbstractListModel, public ObserverT
{
public:
  using item_type = T;
  explicit ItemModelAdapter(Scene& scene);
  virtual ~ItemModelAdapter();
  Qt::DropActions supportedDragActions() const override;
  Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData( const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent ) const override;
  bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent ) override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList &indexes) const override;
  Scene& scene() const;
  virtual T& item_at(const QModelIndex& index) const = 0;

private:
  Scene& m_scene;
};

}  // namespace omm
