#include "managers/itemmodeladapter.h"
#include "scene/contextes.h"
#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/movecommand.h"
#include "commands/copycommand.h"

namespace
{

template<typename T>
bool can_move_drop_items(const std::vector<typename omm::Contextes<T>::Move>& contextes)
{
  const auto is_strictly_valid = [](const typename omm::Contextes<T>::Move& context) {
    return context.is_strictly_valid();
  };

  const auto is_valid = [](const typename omm::Contextes<T>::Move& context) {
    return context.is_valid();
  };

  // all contextes may be moved and at least one move is no noop
  return std::all_of(contextes.begin(), contextes.end(), is_valid)
      && std::any_of(contextes.begin(), contextes.end(), is_strictly_valid);
}

template<typename ContextT, typename ObserverT>
typename std::enable_if<ContextT::is_tree_context, std::vector<ContextT>>::type
make_contextes( const omm::ItemModelAdapter<typename ContextT::item_type, ObserverT>& adapter,
                const QMimeData* data, int row, const QModelIndex& parent )
{
  std::vector<ContextT> contextes;
  using T = typename ContextT::item_type;

  auto property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  auto items = property_owner_mime_data->items<T>();
  if (property_owner_mime_data == nullptr || items.size() == 0) {
  if (property_owner_mime_data == nullptr || property_owner_mime_data->objects().size() == 0) {
    return contextes;
  }

  T& new_parent = adapter.item_at(parent);
  const size_t pos = row < 0 ? new_parent.n_children() : row;

  ContextT::remove_internal_children(items);
  contextes.reserve(items.size());
  const T* predecessor = (pos == 0) ? nullptr : &new_parent.child(pos - 1);
  for (T* subject : items) {
    contextes.emplace_back(*subject, new_parent, predecessor);
    predecessor = subject;
  }

  return contextes;
}

template<typename ContextT, typename ObserverT>
typename std::enable_if<!ContextT::is_tree_context, std::vector<ContextT>>::type
make_contextes( const omm::ItemModelAdapter<typename ContextT::item_type, ObserverT>& adapter,
                const QMimeData* data, int row, const QModelIndex& parent )
{
  std::vector<ContextT> contextes;
  using T = typename ContextT::item_type;

  auto property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  auto items = property_owner_mime_data->items<T>();
  if (property_owner_mime_data == nullptr || items.size() == 0) {
    return contextes;
  }

  LOG(INFO) << "List";
  LOG(INFO) << items.size();
  ContextT::remove_internal_children(items);
  contextes.reserve(items.size());
  const T* predecessor = nullptr; // TODO
  for (T* subject : items) {
    contextes.emplace_back(*subject, predecessor);
    predecessor = subject;
  }
  LOG(INFO) << contextes.size();
  return contextes;
}

}  // namespace

namespace omm
{

template<typename T, typename ObserverT>
ItemModelAdapter<T, ObserverT>::ItemModelAdapter(Scene& scene)
  : m_scene(scene)
{
  m_scene.Observed<ObserverT>::register_observer(*this);
}

template<typename T, typename ObserverT>
ItemModelAdapter<T, ObserverT>::~ItemModelAdapter()
{
  m_scene.Observed<ObserverT>::unregister_observer(*this);
}

template<typename T, typename ObserverT> Qt::DropActions
ItemModelAdapter<T, ObserverT>::supportedDragActions() const
{
  return Qt::LinkAction | Qt::MoveAction | Qt::CopyAction;
}

template<typename T, typename ObserverT> Qt::DropActions
ItemModelAdapter<T, ObserverT>::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}

template<typename T, typename ObserverT> bool ItemModelAdapter<T, ObserverT>
::canDropMimeData( const QMimeData *data, Qt::DropAction action,
                   int row, int column, const QModelIndex &parent ) const
{
  using Context = typename Contextes<T>::Move;
  switch (action) {
  case Qt::MoveAction:
    return data->hasFormat(PropertyOwnerMimeData::MIME_TYPE)
        && qobject_cast<const PropertyOwnerMimeData*>(data) != nullptr
        && can_move_drop_items<T>(make_contextes<Context, ObserverT>(*this, data, row, parent));
  case Qt::CopyAction:
    return data->hasFormat(PropertyOwnerMimeData::MIME_TYPE)
        && qobject_cast<const PropertyOwnerMimeData*>(data) != nullptr;
  default:
    return false;
  }
}

template<typename T, typename ObserverT>
bool ItemModelAdapter<T, ObserverT>::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent )
{
  using MoveContext = typename Contextes<T>::Move;
  using OwningContext = typename Contextes<T>::Owning;
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  } else {
    std::unique_ptr<Command> command;
    switch (action) {
    case Qt::MoveAction: {
      auto move_contextes = make_contextes<MoveContext, ObserverT>(*this, data, row, parent);
      m_scene.submit<MoveCommand<T>>(m_scene, move_contextes);
      break;
    }
    case Qt::CopyAction: {
      auto copy_contextes = make_contextes<OwningContext, ObserverT>(*this, data, row, parent);
      m_scene.submit<CopyCommand<T>>(m_scene, std::move(copy_contextes));
      break;
    }
    }
    return true;
  }
}

template<typename T, typename ObserverT>
QStringList ItemModelAdapter<T, ObserverT>::mimeTypes() const
{
  return { PropertyOwnerMimeData::MIME_TYPE };
}

template<typename T, typename ObserverT>
QMimeData* ItemModelAdapter<T, ObserverT>::mimeData(const QModelIndexList &indexes) const
{
  if (indexes.isEmpty()) {
    return nullptr;
  } else {
    const auto f = [this](const QModelIndex& index) {
      // TODO also return selected tags ans styles
      return &this->item_at(index);
    };
    const auto items = ::transform<AbstractPropertyOwner*, std::vector>(indexes, f);
    return std::make_unique<PropertyOwnerMimeData>(items).release();
  }
}

template<typename T, typename ObserverT>
Scene& ItemModelAdapter<T, ObserverT>::scene() const
{
  return m_scene;
}

template class ItemModelAdapter<Object, AbstractObjectTreeObserver>;
template class ItemModelAdapter<Style, AbstractStyleListObserver>;

}  // namespace omm
