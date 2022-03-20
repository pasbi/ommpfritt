#include "scene/itemmodeladapter.h"
#include "commands/copycommand.h"
#include "commands/movecommand.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include "scene/propertyownermimedata.h"
#include "scene/scene.h"
#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <type_traits>

namespace
{
template<typename item_type, typename StructureT>
bool can_move_drop_items(StructureT& structure,
                         const std::deque<typename omm::Contextes<item_type>::Move>& contextes)
{
  using move_context_type = typename omm::Contextes<item_type>::Move;
  const auto is_strictly_valid = [&structure](const move_context_type& context) {
    return context.is_strictly_valid(structure);
  };

  const auto is_valid
      = [](const typename omm::Contextes<item_type>::Move& context) { return context.is_valid(); };

  // all contextes may be moved and at least one move is no noop
  return std::all_of(contextes.begin(), contextes.end(), is_valid)
         && std::any_of(contextes.begin(), contextes.end(), is_strictly_valid);
}

template<typename ContextT, typename ItemModelAdapterT> requires ContextT::is_tree_context
std::deque<ContextT>
make_contextes(const ItemModelAdapterT& adapter,
               const QMimeData* data,
               int row,
               const QModelIndex& parent)
{
  std::deque<ContextT> contextes;
  using item_type = typename ContextT::item_type;

  const auto* property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  if (property_owner_mime_data == nullptr) {
    return contextes;
  }
  auto items = util::transform<std::set>(property_owner_mime_data->items<item_type>());
  if (items.empty()) {
    return contextes;
  }

  item_type& new_parent = adapter.item_at(parent);
  const std::size_t pos = row < 0 ? new_parent.n_children() : static_cast<std::size_t>(row);
  omm::Object::remove_internal_children(items);
  const auto sorted_items = ContextT::item_type::sort(items);
  const item_type* predecessor = (pos == 0) ? nullptr : &new_parent.tree_child(pos - 1);
  for (item_type* subject : sorted_items) {
    contextes.emplace_back(*subject, new_parent, predecessor);
  }

  return contextes;
}

template<typename ContextT, typename ItemModelAdapterT> requires (!ContextT::is_tree_context)
std::deque<ContextT> make_contextes(const ItemModelAdapterT& adapter,
                                    const QMimeData* data,
                                    int row,
                                    const QModelIndex& parent)
{
  if (parent.isValid()) {
    return std::deque<ContextT>();  // it's a list, not a tree.
  }

  using item_type = typename ContextT::item_type;

  const auto* property_owner_mime_data = qobject_cast<const omm::PropertyOwnerMimeData*>(data);
  if (property_owner_mime_data == nullptr) {
    return {};
  }
  const auto items = property_owner_mime_data->items<item_type>();
  if (items.empty()) {
    return {};
  }

  std::deque<ContextT> contextes;
  const std::size_t pos = row < 0 ? adapter.rowCount() : row;
  const item_type* predecessor
      = (pos == 0) ? nullptr : &adapter.item_at(adapter.index(pos - 1, 0, parent));
  for (item_type* subject : items) {
    contextes.emplace_back(*subject, predecessor);
    predecessor = subject;
  }
  return contextes;
}

bool model_index_tree_position_compare(QModelIndex a, QModelIndex b)
{
  assert(a.isValid());
  assert(b.isValid());

  const auto find_ancestors = [](QModelIndex index) {
    std::vector<QModelIndex> ancestors;
    while (index.isValid()) {
      ancestors.push_back(index);
      index = index.parent();
    }
    return ancestors;
  };

  std::vector<QModelIndex> a_descendants = find_ancestors(a);
  std::vector<QModelIndex> b_descendants = find_ancestors(b);

  for (const auto& item_a : a_descendants) {
    for (const auto& item_b : b_descendants) {
      if (item_a.parent() == item_b.parent()) {
        return item_a.row() < item_b.row();
      }
    }
  }
  Q_UNREACHABLE();
  return false;
}

}  // namespace

namespace omm
{
template<typename StructureT, typename ItemT, typename ItemModel>
ItemModelAdapter<StructureT, ItemT, ItemModel>::ItemModelAdapter(Scene& scene,
                                                                 StructureT& structure)
    : scene(scene), structure(structure)
{
}

template<typename StructureT, typename ItemT, typename ItemModel>
Qt::DropActions ItemModelAdapter<StructureT, ItemT, ItemModel>::supportedDragActions() const
{
  return Qt::LinkAction | Qt::MoveAction | Qt::CopyAction;
}

template<typename StructureT, typename ItemT, typename ItemModel>
Qt::DropActions ItemModelAdapter<StructureT, ItemT, ItemModel>::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}

template<typename StructureT, typename ItemT, typename ItemModel>
bool ItemModelAdapter<StructureT, ItemT, ItemModel>::canDropMimeData(
    const QMimeData* data,
    Qt::DropAction action,
    int row,
    int column,
    const QModelIndex& parent) const
{
  Q_UNUSED(column);
  if (!data->hasFormat(PropertyOwnerMimeData::MIME_TYPE)) {
    return false;
  }
  const auto* pdata = qobject_cast<const PropertyOwnerMimeData*>(data);
  if (pdata == nullptr) {
    return false;
  }
  const auto items = pdata->items<typename structure_type::item_type>();
  if (items.empty()) {
    return false;
  }

  switch (action) {
  case Qt::MoveAction: {
    using Context = typename Contextes<typename structure_type::item_type>::Move;
    const auto move_contextes = make_contextes<Context>(*this, data, row, parent);
    return can_move_drop_items<typename structure_type::item_type>(structure, move_contextes);
  }
  case Qt::CopyAction:
    return true;
  default:
    return false;
  }
}

template<typename StructureT, typename ItemT, typename ItemModel>
bool ItemModelAdapter<StructureT, ItemT, ItemModel>::dropMimeData(const QMimeData* data,
                                                                  Qt::DropAction action,
                                                                  int row,
                                                                  int column,
                                                                  const QModelIndex& parent)
{
  using MoveContext = typename Contextes<typename structure_type::item_type>::Move;
  using OwningContext = typename Contextes<typename structure_type::item_type>::Owning;
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  } else {
    std::unique_ptr<Command> command;
    switch (action) {
    case Qt::MoveAction: {
      auto move_contextes = make_contextes<MoveContext>(*this, data, row, parent);
      if (move_contextes.empty()) {
        return false;
      }
      scene.submit<MoveCommand<StructureT>>(structure, move_contextes);
      break;
    }
    case Qt::CopyAction: {
      auto copy_contextes = make_contextes<OwningContext>(*this, data, row, parent);
      if (copy_contextes.empty()) {
        return false;
      }
      scene.submit<CopyCommand<StructureT>>(structure, std::move(copy_contextes));
      break;
    }
    default:
      break;
    }
    return true;
  }
}

template<typename StructureT, typename ItemT, typename ItemModel>
QStringList ItemModelAdapter<StructureT, ItemT, ItemModel>::mimeTypes() const
{
  return {PropertyOwnerMimeData::MIME_TYPE};
}

template<typename StructureT, typename ItemT, typename ItemModel>
QMimeData*
ItemModelAdapter<StructureT, ItemT, ItemModel>::mimeData(const QModelIndexList& indexes) const
{
  if (indexes.isEmpty()) {
    return nullptr;
  } else {
    const auto f = [this](const QModelIndex& index) {
      return static_cast<AbstractPropertyOwner*>(&this->item_at(index));
    };

    auto sorted_indexes = indexes;
    // TODO replace this very inefficient approach with topological_context_sort
    std::sort(sorted_indexes.begin(), sorted_indexes.end(), model_index_tree_position_compare);
    const auto items = util::transform<std::vector>(sorted_indexes, f);
    return std::make_unique<PropertyOwnerMimeData>(items).release();
  }
}

template class ItemModelAdapter<ObjectTree, Object, QAbstractItemModel>;
template class ItemModelAdapter<StyleList, Style, QAbstractListModel>;

}  // namespace omm
