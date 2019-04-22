#pragma once

#include <memory>
#include <vector>
#include <set>
#include <algorithm>
#include "abstractraiiguard.h"
#include "common.h"

namespace omm
{

template<typename T>
class TreeElement
{
public:
  explicit TreeElement(T* parent);
  virtual ~TreeElement() = default;
  explicit TreeElement(const TreeElement& other);
  bool is_root() const;
  T& parent() const;
  T& adopt(std::unique_ptr<T> adoptee, const size_t pos);
  T& adopt(std::unique_ptr<T> adoptee);
  std::unique_ptr<T> repudiate(T& repudiatee);
  std::vector<T*> children() const;
  T& child(size_t i) const;
  size_t n_children() const;
  bool is_descendant_of(const T& subject) const;
  void reset_parent(T& new_parent);
  std::set<T*> all_descendants() const;
  size_t position() const;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() = 0;

  static void remove_internal_children(std::set<T*>& items);

protected:
  virtual void on_children_changed() {}

private:
  T* m_parent;
  std::vector<std::unique_ptr<T>> m_children;
  T& get() { return static_cast<T&>(*this); }
  const T& get() const { return static_cast<const T&>(*this); }
};

}  // namespace omm
