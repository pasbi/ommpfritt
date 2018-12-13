#include "managers/objectmanager/objecttreeselectionmodel.h"
#include "scene/objecttreeadapter.h"

namespace omm
{

ObjectTreeSelectionModel::ObjectTreeSelectionModel(ObjectTreeAdapter& adapter)
  : QItemSelectionModel(&adapter)
{
}

bool ObjectTreeSelectionModel::is_selected(const QModelIndex& index, Tag& tag) const
{
  return m_selected_tags.count(index) > 0
      && m_selected_tags.at(index).count(&tag) > 0;
}

void ObjectTreeSelectionModel::select( const QModelIndex& index, Tag& tag,
                                       QItemSelectionModel::SelectionFlags command )
{
  const bool is_selected = m_selected_tags[index].count(&tag);
  if (command & QItemSelectionModel::Clear) {
    clear_selection();
  }

  if (command & QItemSelectionModel::Select) {
    m_selected_tags[index].insert(&tag);
  } else if (command & QItemSelectionModel::Deselect) {
    m_selected_tags[index].erase(&tag);
  } else if (command & QItemSelectionModel::Toggle) {
    if (is_selected) {
      select(index, tag, QItemSelectionModel::Deselect);
    } else {
      select(index, tag, QItemSelectionModel::Select);
    }
  }

  if (m_selected_tags[index].size() == 0) {
    m_selected_tags.erase(index);
  }
}

void ObjectTreeSelectionModel::clear_selection()
{
  QItemSelectionModel::clearSelection();
  m_selected_tags.clear();
}

void ObjectTreeSelectionModel::select( const QModelIndex &index,
                                       QItemSelectionModel::SelectionFlags command)
{
  if (command & QItemSelectionModel::Clear) {
    LOG(INFO) << "clear";
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(index, command);
}

void ObjectTreeSelectionModel::select( const QItemSelection &selection,
                                       QItemSelectionModel::SelectionFlags command)
{
  if (command & QItemSelectionModel::Clear) {
    LOG(INFO) << "clear";
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(selection, command);
}

std::set<Tag*> ObjectTreeSelectionModel::selected_tags() const
{
  std::set<Tag*> selection;
  for (const auto& it : m_selected_tags) {
    selection = ::merge(selection, it.second);
  }
  return selection;
}



}  // namespace omm
