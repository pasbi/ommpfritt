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

  void insert(std::unique_ptr<T> t, const T* predecessor);
  void insert(ListOwningContext<T>& context);
  void remove(ListOwningContext<T>& t);
  void move(ListMoveContext<T>& context);
  T& item(size_t i) const;
  std::vector<std::unique_ptr<T>> set(std::vector<std::unique_ptr<T>> items);

  std::set<T*> items() const override;
  const T* predecessor(const T& sibling) const override;
  size_t position(const T& item) const;

  T& insert(std::unique_ptr<T> t) override;
  std::unique_ptr<T> remove(T& t) override;

  void invalidate();

private:
  std::vector<std::unique_ptr<Style>> m_items;
};

}  // namespace omm
