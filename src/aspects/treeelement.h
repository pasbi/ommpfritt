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
  virtual ~TreeElement();
  explicit TreeElement(const TreeElement& other);
  bool is_root() const;
  T& parent() const;
  T& adopt(std::unique_ptr<T> adoptee, const size_t pos);
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

  template<template<typename...> class Container>
  static void remove_internal_children(Container<T*>& items)
  {
    const auto has_parent = [&items](const T* subject) {
      const auto predicate = [subject](const T* potential_descendant) {
        return potential_descendant != subject && potential_descendant->is_descendant_of(*subject);
      };
      return std::any_of(items.begin(), items.end(), predicate);
    };

    ::erase_if(items, [&has_parent](const T* subject) { return has_parent(subject); });
  }

private:
  std::vector<std::unique_ptr<T>> m_children;
  T* m_parent;
  T& get() { return static_cast<T&>(*this); }
  const T& get() const { return static_cast<const T&>(*this); }
};

}  // namespace omm