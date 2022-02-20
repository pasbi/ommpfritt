#pragma once

#include "scene/contextes_fwd.h"
#include "scene/structure.h"
#include <memory>
#include <set>
#include <deque>

namespace omm
{
template<typename T> class List : public Structure<T>
{
public:
  constexpr static bool is_tree = false;
  using Structure<T>::Structure;
  explicit List(const List<T>& other);
  ~List() = default;
  List& operator=(const List&) = delete;
  List& operator=(List&&) = delete;
  List(List&&) = delete;
  virtual void insert(ListOwningContext<T>& context);
  virtual void remove(ListOwningContext<T>& t);
  std::unique_ptr<T> remove(T& item) override;
  virtual void move(ListMoveContext<T>& context);
  T& item(std::size_t i) const;
  virtual std::deque<std::unique_ptr<T>> set(std::deque<std::unique_ptr<T> > items);

  std::set<T*> items() const override;
  std::deque<T*> ordered_items() const;
  const T* predecessor(const T& item) const override;
  using Structure<T>::predecessor;
  std::size_t position(const T& item) const override;
  std::size_t insert_position(const T* predecessor) const override;
  [[nodiscard]] std::size_t size() const;
  bool contains(const T& item) const;

private:
  std::deque<std::unique_ptr<T>> m_items;
};

}  // namespace omm
