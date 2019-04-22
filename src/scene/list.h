#pragma once

#include <vector>
#include <memory>
#include <set>
#include "scene/contextes_fwd.h"
#include "scene/structure.h"
#include "scene/abstractstructureobserver.h"
#include "observed.h"

namespace omm
{

template<typename T>
class List : public Structure<T>, public Observed<AbstractStructureObserver<List<T>>>
{
public:
  using observer_type = AbstractStructureObserver<List<T>>;
  using observed_type = Observed<observer_type>;
  constexpr static bool is_tree = false;
  using Structure<T>::Structure;
  explicit List(const List<T>& other);
  void insert(ListOwningContext<T>& context);
  void remove(ListOwningContext<T>& t);
  void move(ListMoveContext<T>& context);
  T& item(size_t i) const;
  std::vector<std::unique_ptr<T>> set(std::vector<std::unique_ptr<T>> items);

  std::set<T*> items() const override;
  std::vector<T*> ordered_items() const;
  const T* predecessor(const T& sibling) const override;
  using Structure<T>::predecessor;
  size_t position(const T& item) const override;
  size_t size() const;

  std::unique_ptr<T> remove(T& t) override;

  void invalidate() override;
  bool contains(const T& item) const;

private:
  std::vector<std::unique_ptr<T>> m_items;
};

}  // namespace omm
