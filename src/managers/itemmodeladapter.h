#pragma once

#include <QAbstractItemModel>

namespace omm
{

class Scene;

template<typename ObserverT>
class ItemModelAdapter : public ObserverT::item_model, public ObserverT
{
  static_assert( std::is_base_of<QAbstractItemModel, typename ObserverT::item_model>::value,
                 "BaseModel must be derived from QAbstractItemModel" );
public:
  using item_type = typename ObserverT::item_type;
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
  virtual item_type& item_at(const QModelIndex& index) const = 0;

private:
  Scene& m_scene;
};

}  // namespace omm
