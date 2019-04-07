#include "aspects/treeelement.h"
#include "common.h"
#include "objects/object.h"

namespace omm
{

template<typename T> TreeElement<T>::TreeElement(T* parent) : m_parent(parent)
{
  static_assert(std::is_base_of<TreeElement<T>, T>::value, "T must derive ElementType<T>");
}

template<typename T> TreeElement<T>::TreeElement(const TreeElement& other)
  : m_parent(nullptr)
  , m_children(::copy(other.m_children))
{
  for (auto&& child : m_children) {
    child->m_parent = static_cast<T*>(this);
  }
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object, const size_t pos)
{
  assert(object->is_root());
  auto guard = object->acquire_set_parent_guard();
  object->m_parent = &get();
  return insert(m_children, std::move(object), pos);
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object)
{
  return adopt(std::move(object), n_children());
}

template<typename T> std::unique_ptr<T> TreeElement<T>::repudiate(T& object)
{
  auto guard = object.acquire_set_parent_guard();
  object.m_parent = nullptr;
  std::unique_ptr<T> optr = extract(m_children, object);
  return optr;
}

template<typename T> void TreeElement<T>::reset_parent(T& new_parent)
{
  assert(!is_root()); // use Object::adopt for roots.
  new_parent.adopt(m_parent->repudiate(get()));
}

template<typename T> std::vector<T*> TreeElement<T>::children() const
{
  return ::transform<T*>(m_children, [](const auto& up) { return up.get(); });
}

template<typename T> size_t TreeElement<T>::n_children() const { return m_children.size(); }
template<typename T> T& TreeElement<T>::child(size_t i) const { return *m_children[i]; }
template<typename T> bool TreeElement<T>::is_root() const { return m_parent == nullptr; }

template<typename T> T& TreeElement<T>::parent() const
{
  assert(!is_root());
  return *m_parent;
}

template<typename T> bool TreeElement<T>::is_descendant_of(const T& subject) const
{
  if (&subject == this) {
    return true;
  } else if (subject.is_root()) {
    return false;
  } else {
    return is_descendant_of(subject.parent());
  }
}

template<typename T> std::set<T*> TreeElement<T>::all_descendants() const
{
  const auto children = this->children();
  std::set<T*> all_descendants(children.begin(), children.end());
  for (const auto& child : children) {
    const auto child_descendants = child->all_descendants();
    all_descendants.insert(child_descendants.begin(), child_descendants.end());
  }
  return all_descendants;
}

template<typename T> size_t TreeElement<T>::position() const
{
  assert (!is_root());
  const auto siblings = parent().children();
  const auto it = std::find(siblings.begin(), siblings.end(), this);
  assert(it != siblings.end());
  return std::distance(siblings.begin(), it);
}

template class TreeElement<Object>;

}  // namespace omm
