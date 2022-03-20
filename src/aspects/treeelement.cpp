#include "aspects/treeelement.h"
#include "common.h"
#include "objects/object.h"
#include <QString>

namespace omm
{
template<typename T>
TreeElement<T>::TreeElement(const TreeElement& other)
    : m_parent(nullptr), m_children(::copy(other.m_children))
{
  static_assert(std::is_base_of_v<TreeElement<T>, T>, "T must derive ElementType<T>");
  for (auto&& child : m_children) {
    child->m_parent = static_cast<T*>(this);
  }
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object, const std::size_t pos)
{
  assert(object->is_root());
  object->m_parent = &get();
  auto& r = insert(m_children, std::move(object), pos);
  on_child_added(r);
  return r;
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object)
{
  return adopt(std::move(object), n_children());
}

template<typename T> std::unique_ptr<T> TreeElement<T>::repudiate(T& object)
{
  object.m_parent = nullptr;
  std::unique_ptr<T> optr = extract(m_children, object);
  on_child_removed(object);
  return optr;
}

template<typename T> void TreeElement<T>::reset_parent(T& new_parent)
{
  assert(!is_root());  // use Object::adopt for roots.
  new_parent.adopt(m_parent->repudiate(get()));
}

template<typename T> std::vector<T*> TreeElement<T>::tree_children() const
{
  return util::transform(m_children, [](const auto& up) { return up.get(); });
}

template<typename T> std::size_t TreeElement<T>::n_children() const
{
  return m_children.size();
}
template<typename T> T& TreeElement<T>::tree_child(std::size_t i) const
{
  return *m_children[i];
}
template<typename T> bool TreeElement<T>::is_root() const
{
  return m_parent == nullptr;
}

template<typename T> T& TreeElement<T>::tree_parent() const
{
  assert(!is_root());
  return *m_parent;
}

template<typename T> bool TreeElement<T>::is_ancestor_of(const T& subject) const
{
  if (&subject == this) {
    return true;
  } else if (subject.is_root()) {
    return false;
  } else {
    return is_ancestor_of(subject.tree_parent());
  }
}

template<typename T> std::set<T*> TreeElement<T>::all_descendants() const
{
  const auto children = this->tree_children();
  std::set<T*> all_descendants(children.begin(), children.end());
  for (const auto& child : children) {
    const auto child_descendants = child->all_descendants();
    all_descendants.insert(child_descendants.begin(), child_descendants.end());
  }
  return all_descendants;
}

template<typename T> std::size_t TreeElement<T>::position() const
{
  assert(!is_root());
  const auto siblings = tree_parent().tree_children();
  const auto it = std::find(siblings.begin(), siblings.end(), this);
  assert(it != siblings.end());
  return std::distance(siblings.begin(), it);
}

template<typename T> void TreeElement<T>::remove_internal_children(std::set<T*>& items)
{
  auto i = items.begin();
  while (i != items.end()) {
    const bool hit = std::any_of(items.begin(), items.end(), [&i](const T* potential_descendant) {
      return potential_descendant != *i && potential_descendant->is_ancestor_of(**i);
    });
    if (hit) {
      i = items.erase(i);
    } else {
      std::advance(i, 1);
    }
  }
}

template<typename T> T* TreeElement<T>::lowest_common_ancestor(T* a, T* b)
{
  // This is a rather inefficient implementation (O(n^2) where n is depth of the tree)
  // at O(1) with O(n) preparation is possible but apparently hard to implement.
  T* candidate = a;
  if (candidate->is_ancestor_of(*b)) {
    return candidate;
  }
  while (!candidate->is_root()) {
    candidate = &candidate->tree_parent();
    if (candidate->is_ancestor_of(*b)) {
      return candidate;
    }
  }
  return nullptr;
}

template<typename T> const T* TreeElement<T>::lowest_common_ancestor(const T* a, const T* b)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<const T*>(lowest_common_ancestor(const_cast<T*>(a), const_cast<T*>(b)));
}

template<typename T> std::deque<T*> TreeElement<T>::sort(const std::set<T*>& items)
{
  std::deque vs(items.begin(), items.end());
  std::sort(vs.begin(), vs.end(), tree_gt<T>);
  return vs;
}

template class TreeElement<Object>;
template class TreeElement<TreeTestItem>;

}  // namespace omm
