#pragma once

#include <vector>
#include <memory>
#include <set>
#include "scene/contextes_fwd.h"
#include "scene/structure.h"
#include "aspects/propertyowner.h"
#include "observed.h"

namespace omm
{

template<typename T>
class List
    : public Structure<T>
    , public AbstractPropertyOwnerObserver
{
public:
  constexpr static bool is_tree = false;
  using Structure<T>::Structure;
  explicit List(const List<T>& other);
  ~List();
  virtual void insert(ListOwningContext<T>& context);
  virtual void remove(ListOwningContext<T>& t);
  std::unique_ptr<T> remove(T& t) override;
  virtual void move(ListMoveContext<T>& context);
  T& item(size_t i) const;
  virtual std::vector<std::unique_ptr<T>> set(std::vector<std::unique_ptr<T>> items);

  std::set<T*> items() const override;
  std::vector<T*> ordered_items() const;
  const T* predecessor(const T& sibling) const override;
  using Structure<T>::predecessor;
  size_t position(const T& item) const override;
  size_t size() const;


  bool contains(const T& item) const;

  void on_change(AbstractPropertyOwner *apo, int what, Property *property,
                 std::set<const void*> trace) override;

private:
  std::vector<std::unique_ptr<T>> m_items;
};

}  // namespace omm
