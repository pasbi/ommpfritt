#include "scene/listadapter.h"
#include "renderers/style.h"
#include "scene/scene.h"

namespace omm
{

template<typename ItemT> ListAdapter<ItemT>::ListAdapter(Scene& scene, List<ItemT>& list)
  : ItemModelAdapter<List<ItemT>, QAbstractListModel>(scene, list)
  , list_structure(list)
{
}

template<typename ItemT> int ListAdapter<ItemT>::rowCount(const QModelIndex& parent) const
{
  assert(!parent.isValid());
  return this->structure.items().size();
}

template<typename ItemT> QVariant ListAdapter<ItemT>::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  assert(!index.parent().isValid());

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return QString::fromStdString(this->structure.item(index.row()).name());
  case Qt::DecorationRole:
    return this->structure.item(index.row()).icon();
  }
  return QVariant();
}

template<typename ItemT> Qt::ItemFlags ListAdapter<ItemT>::flags(const QModelIndex &index) const
{
  assert(!index.parent().isValid());
  return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren;
}

template<typename ItemT> std::unique_ptr<AbstractRAIIGuard>
ListAdapter<ItemT>::acquire_inserter_guard(int row)
{
  class InserterGuard : public AbstractRAIIGuard
  {
  public:
    InserterGuard(ListAdapter& model, int row) : m_model(model)
    {
      m_model.beginInsertRows(QModelIndex(), row, row);
    }
    ~InserterGuard() { m_model.endInsertRows(); }
  private:
    ListAdapter& m_model;
  };
  return std::make_unique<InserterGuard>(*this, row);
}

  // friend class AbstractRAIIGuard;
  // std::unique_ptr<AbstractRAIIGuard> acquire_mover_guard() override;

template<typename ItemT> std::unique_ptr<AbstractRAIIGuard>
ListAdapter<ItemT>::acquire_remover_guard(int row)
{
  class RemoverGuard : public AbstractRAIIGuard
  {
  public:
    RemoverGuard(ListAdapter& model, int row) : m_model(model)
    {
      m_model.beginRemoveRows(QModelIndex(), row, row);
    }
    ~RemoverGuard() { m_model.endRemoveRows(); }
  private:
    ListAdapter& m_model;
  };
  return std::make_unique<RemoverGuard>(*this, row);
}

template<typename ItemT> std::unique_ptr<AbstractRAIIGuard>
ListAdapter<ItemT>::acquire_reseter_guard()
{
  class ReseterGuard : public AbstractRAIIGuard
  {
  public:
    ReseterGuard(ListAdapter& model) : m_model(model)
    {
      m_model.beginResetModel();
    }
    ~ReseterGuard() { m_model.endResetModel(); }
  private:
    ListAdapter& m_model;
  };
  return std::make_unique<ReseterGuard>(*this);
}

template<typename ItemT> std::unique_ptr<AbstractRAIIGuard>
ListAdapter<ItemT>::acquire_mover_guard(const ListMoveContext<ItemT>& context)
{
  class MoverGuard : public AbstractRAIIGuard
  {
  public:
    MoverGuard(ListAdapter& model, int old_pos, int new_pos)
      : m_model(model)
    {
      m_model.beginMoveRows(QModelIndex(), old_pos, old_pos, QModelIndex(), new_pos);
    }

    ~MoverGuard() { m_model.endMoveRows(); }
  private:
    ListAdapter& m_model;
  };

  const auto old_pos = list_structure.position(context.subject);
  const auto new_pos = list_structure.insert_position(context.predecessor);

  if (old_pos == new_pos) {
    return nullptr;
  } else {
    return std::make_unique<MoverGuard>(*this, old_pos, new_pos);
  }
}

template<typename ItemT>
bool ListAdapter<ItemT>::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  assert(index.column() == 0);
  assert(!index.parent().isValid());

  auto& item = this->structure.item(index.row());
  auto& name_property = item.property(Object::NAME_PROPERTY_KEY);
  name_property.set(value.toString().toStdString());
  return true;
}

template<typename ItemT> ItemT& ListAdapter<ItemT>::item_at(const QModelIndex& index) const
{
  assert(index.isValid());
  assert(!index.parent().isValid());
  return this->structure.item(index.row());
}

template class ListAdapter<Style>;

}  // namespace omm
