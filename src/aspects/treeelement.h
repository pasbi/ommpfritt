#pragma once

#include <memory>
#include <vector>
#include <set>
#include "abstractraiiguard.h"

namespace omm
{

template<typename T>
class TreeElement
{
public:
  explicit TreeElement(T* parent);
  virtual ~TreeElement();
  bool is_root() const;
  T& parent() const;
  T& adopt(std::unique_ptr<T> adoptee, const T* predecessor);
  T& adopt(std::unique_ptr<T> adoptee);
  std::unique_ptr<T> repudiate(T& repudiatee);
  std::vector<T*> children() const;
  T& child(size_t i) const;
  const T* predecessor() const;
  size_t n_children() const;
  bool is_descendant_of(const T& subject) const;
  void reset_parent(T& new_parent);
  std::set<T*> all_descendants() const;
  size_t position() const;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() = 0;

private:
  std::vector<std::unique_ptr<T>> m_children;
  T* m_parent;
  T& get() { return static_cast<T&>(*this); }
  const T& get() const { return static_cast<const T&>(*this); }
};

}  // namespace omm