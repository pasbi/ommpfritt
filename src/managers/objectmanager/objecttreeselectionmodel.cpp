#include "managers/objectmanager/objecttreeselectionmodel.h"
#include "scene/scene.h"
#include "tags/tag.h"
#include <stack>

namespace omm
{
ObjectTreeSelectionModel::ObjectTreeSelectionModel(ObjectTree& adapter)
    : QItemSelectionModel(&adapter)
{
}

bool ObjectTreeSelectionModel::is_selected(Tag& tag) const
{
  return m_selected_tags.contains(&tag);
}

void ObjectTreeSelectionModel::select(Tag& tag, QItemSelectionModel::SelectionFlags command)
{
  m_current_tag = &tag;
  if ((command & QItemSelectionModel::Clear) != 0u) {
    clear_selection();
  }

  if ((command & QItemSelectionModel::Select) != 0u) {
    m_selected_tags.insert(&tag);
  } else if ((command & QItemSelectionModel::Deselect) != 0u) {
    m_selected_tags.erase(&tag);
  } else if ((command & QItemSelectionModel::Toggle) != 0u) {
    select(tag, is_selected(tag) ? QItemSelectionModel::Deselect : QItemSelectionModel::Select);
  }
}

void ObjectTreeSelectionModel::clear_selection()
{
  QItemSelectionModel::clearSelection();
  m_selected_tags.clear();
}

void ObjectTreeSelectionModel::select(const QModelIndex& index,
                                      QItemSelectionModel::SelectionFlags command)
{
  const bool is_tag_index = index.column() == omm::ObjectTree::TAGS_COLUMN;
  if (((command & QItemSelectionModel::Clear) != 0u) && !is_tag_index) {
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(index, command);
}

void ObjectTreeSelectionModel::select(const QItemSelection& selection,
                                      QItemSelectionModel::SelectionFlags command)
{
  const auto has_tag_index = [](const QItemSelectionRange& range) {
    return range.left() <= ObjectTree::TAGS_COLUMN && range.right() >= ObjectTree::TAGS_COLUMN;
  };
  const bool selection_has_tags = std::any_of(selection.begin(), selection.end(), has_tag_index);

  if (((command & QItemSelectionModel::Clear) != 0u) && !selection_has_tags) {
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(selection, command);
}

std::set<Tag*> ObjectTreeSelectionModel::selected_tags() const
{
  return m_selected_tags;
}

std::vector<Tag*> ObjectTreeSelectionModel::selected_tags_ordered(Scene& scene) const
{
  std::list<Tag*> selected_tags;
  std::stack<Object*> stack;
  stack.push(&scene.object_tree().root());
  while (!stack.empty()) {
    Object* object = stack.top();
    stack.pop();

    for (Tag* tag : object->tags.ordered_items()) {
      if (m_selected_tags.contains(tag)) {
        selected_tags.push_back(tag);
      }
    }

    for (Object* child : object->tree_children()) {
      stack.push(child);
    }
  }

  return std::vector(selected_tags.begin(), selected_tags.end());
}

void ObjectTreeSelectionModel::extend_selection(Tag& tag)
{
  if (m_current_tag == nullptr || m_current_tag->owner != tag.owner) {
    select(tag, QItemSelectionModel::Select);
  } else {
    const auto tags = tag.owner->tags.ordered_items();
    auto begin = std::find(tags.begin(), tags.end(), &tag);
    auto end = std::find(tags.begin(), tags.end(), m_current_tag);
    assert(begin != tags.end() && end != tags.end());
    if (end < begin) {  // NOLINT(modernize-use-nullptr)
      std::swap(begin, end);
    }
    std::advance(end, 1);
    for (auto it = begin; it != end; ++it) {
      select(**it, QItemSelectionModel::Select);
    }
  }
}

const ObjectTree& ObjectTreeSelectionModel::model() const
{
  return dynamic_cast<const ObjectTree&>(*QItemSelectionModel::model());
}

void ObjectTreeSelectionModel::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  m_selected_tags = kind_cast<Tag>(selection);
  QItemSelection new_selection;
  for (Object* object : kind_cast<Object>(selection)) {
    QModelIndex index = model().index_of(*object);
    new_selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
  }
  QItemSelectionModel::select(new_selection, QItemSelectionModel::ClearAndSelect);
}

}  // namespace omm
