#include "scene/tree.h"
#include "scene/contextes.h"
#include <QTimer>
#include <type_traits>
#include "scene/scene.h"
#include "objecttreeadapter.h"

namespace omm
{

template<typename T> Tree<T>::Tree(std::unique_ptr<T> root, Scene& scene)
  : Structure<T>(), m_root(std::move(root)), m_scene(scene)
{
}

template<typename T> T& Tree<T>::root() const
{
  return *m_root;
}

template<typename T> bool Tree<T>::contains(const T& t) const
{
  static_assert(std::is_base_of_v<TreeElement<T>, T>);
  const auto& root = static_cast<const TreeElement<T>&>(*m_root);
  return root.is_ancestor_of(t);
}

template<typename T> void Tree<T>::move(TreeMoveContext<T>& context)
{
  assert(context.is_valid());

  Object& old_parent = context.subject.get().tree_parent();
  Object& new_parent = context.parent.get();
  const auto old_pos = m_scene.object_tree.position(context.subject);
  const auto new_pos = m_scene.object_tree.insert_position(context.predecessor);
  const QModelIndex old_parent_index = m_scene.object_tree_adapter.index_of(old_parent);
  const QModelIndex new_parent_index = m_scene.object_tree_adapter.index_of(new_parent);

  m_scene.object_tree_adapter.beginMoveRows(old_parent_index, old_pos, old_pos,
                                            new_parent_index, new_pos);
  auto item = old_parent.repudiate(context.subject);
  const auto pos = this->insert_position(context.predecessor);
  context.parent.get().adopt(std::move(item), pos);
  m_item_cache_is_dirty = true;
  m_scene.object_tree_adapter.endMoveRows();
  Q_EMIT m_scene.repaint();
}

template<typename T> void Tree<T>::insert(TreeOwningContext<T>& context)
{
  assert(context.subject.owns());

  const auto row = this->insert_position(context.predecessor);
  const QModelIndex parent_index = m_scene.object_tree_adapter.index_of(context.parent);
  m_scene.object_tree_adapter.beginInsertRows(parent_index, row, row);
  context.parent.get().adopt(context.subject.release(), row);
  m_item_cache_is_dirty = true;
  m_scene.object_tree_adapter.endInsertRows();
  Q_EMIT m_scene.repaint();
}

template<typename T> void Tree<T>::remove(TreeOwningContext<T>& context)
{
  assert(!context.subject.owns());
  const Object& subject = context.subject;
  const int row = m_scene.object_tree.position(subject);
  const QModelIndex parent_index = m_scene.object_tree_adapter.index_of(subject.tree_parent());
  m_scene.object_tree_adapter.beginRemoveRows(parent_index, row, row);
  context.subject.capture(context.parent.get().repudiate(context.subject));
  m_item_cache_is_dirty = true;
  m_scene.object_tree_adapter.endRemoveRows();
  Q_EMIT m_scene.repaint();
}

template<typename T> std::unique_ptr<T> Tree<T>::remove(T& t)
{
  const int row = m_scene.object_tree.position(t);
  const QModelIndex parent_index = m_scene.object_tree_adapter.index_of(t.tree_parent());
  m_scene.object_tree_adapter.beginRemoveRows(parent_index, row, row);
  assert(!t.is_root());
  auto item = t.tree_parent().repudiate(t);
  m_item_cache_is_dirty = true;
  m_scene.object_tree_adapter.endRemoveRows();
  Q_EMIT m_scene.repaint();
  return item;
}

template<typename T>
std::unique_ptr<T> Tree<T>::replace_root(std::unique_ptr<T> new_root)
{
  m_scene.object_tree_adapter.beginResetModel();
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  m_item_cache_is_dirty = true;
  m_scene.object_tree_adapter.endResetModel();
  Q_EMIT m_scene.repaint();
  return old_root;
}

template<typename T> std::set<T*> Tree<T>::items() const
{
  if (m_item_cache_is_dirty) {
    m_item_cache_is_dirty = false;
    m_item_cache = root().all_descendants();
  }
  return m_item_cache;
}

template<typename T> size_t Tree<T>::position(const T& item) const
{
  return item.position();
}

template<typename T> const T* Tree<T>::predecessor(const T& sibling) const
{
  assert(!sibling.is_root());
  const auto pos = position(sibling);
  if (pos == 0) {
    return nullptr;
  } else {
    return &sibling.tree_parent().tree_child(pos - 1);
  }
}

template class Tree<Object>;

}  // namespace
