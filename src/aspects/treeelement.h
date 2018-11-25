#pragma once

#include <memory>
#include <vector>

namespace omm
{

template<typename T>
class TreeElement
{
public:
  TreeElement(T* parent);
  virtual ~TreeElement();
  bool is_root() const;
  T& parent() const;
  T& adopt(std::unique_ptr<T> adoptee, const T* predecessor);
  T& adopt(std::unique_ptr<T> adoptee);
  std::unique_ptr<T> repudiate(T& repudiatee);
  std::vector<T*> children() const;
  T& child(size_t i) const;
  size_t row() const;
  const T* predecessor() const;
  size_t n_children() const;
  size_t get_insert_position(const T* child_before_position) const;
  bool is_descendant_of(const T& subject) const;
  void reset_parent(T& new_parent);

private:
  std::vector<std::unique_ptr<T>> m_children;
  T* m_parent;
  T& get() { return static_cast<T&>(*this); }
  const T& get() const { return static_cast<const T&>(*this); }
};

}  // namespace omm