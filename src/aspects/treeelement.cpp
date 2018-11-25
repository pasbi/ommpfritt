#include "aspects/treeelement.h"
#include "common.h"
#include "objects/object.h"

namespace omm
{

template<typename T> TreeElement<T>::TreeElement(T* parent)
  : m_parent(parent)
{
  static_assert(std::is_base_of<TreeElement<T>, T>::value, "T must derive ElementType<T>");
}

template<typename T> TreeElement<T>::~TreeElement()
{
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object, const T* predecessor)
{
  assert(object->is_root());
  // const ObjectTransformation gt = object->global_transformation();  // TODO
  assert(predecessor == nullptr || &predecessor->parent() == this);
  const size_t pos = get_insert_position(predecessor);

  object->m_parent = &get();
  T& oref = insert(m_children, std::move(object), pos);
  // oref.set_global_transformation(gt);  // TODO
  return oref;
}

template<typename T> T& TreeElement<T>::adopt(std::unique_ptr<T> object)
{
  const size_t n = n_children();
  const T* predecessor = n == 0 ? nullptr : &child(n-1);
  return adopt(std::move(object), predecessor);
}

template<typename T> std::unique_ptr<T> TreeElement<T>::repudiate(T& object)
{
  // const ObjectTransformation gt = object.global_transformation();    // TODO
  object.m_parent = nullptr;
  std::unique_ptr<T> optr = extract(m_children, object);
  // object.set_global_transformation(gt);    // TODO
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

template<typename T> size_t TreeElement<T>::n_children() const
{
  return m_children.size();
}

template<typename T> T& TreeElement<T>::child(size_t i) const
{
  return *m_children[i];
}

template<typename T> bool TreeElement<T>::is_root() const
{
  return m_parent == nullptr;
}

template<typename T> T& TreeElement<T>::parent() const
{
  assert(!is_root());
  return *m_parent;
}

template<typename T> size_t TreeElement<T>::row() const
{
  assert (!is_root());
  const auto siblings = parent().children();
  for (size_t i = 0; i < siblings.size(); ++i) {
    if (siblings[i] == &get()) {
      return i;
    }
  }
  assert(false);
}

template<typename T> const T* TreeElement<T>::predecessor() const
{
  assert(!is_root());
  const auto pos = row();
  if (pos == 0) {
    return nullptr;
  } else {
    return &parent().child(pos - 1);
  }
}

template<typename T>
size_t TreeElement<T>::get_insert_position(const T* child_before_position) const
{
  if (child_before_position == nullptr) {
    return 0;
  } else {
    assert(&child_before_position->parent() == this);
    return child_before_position->row() + 1;
  }
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

template class TreeElement<Object>;

}  // namespace omm
