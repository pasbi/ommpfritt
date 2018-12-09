#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdint>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observed.h"
#include "scene/cachedgetter.h"
#include "scene/structure.h"
#include "scene/contextes_fwd.h"
#include "scene/abstractstructureobserver.h"
#include "objects/object.h"

namespace omm
{

template<typename T>
class Tree : public Structure<T>, public Observed<AbstractStructureObserver<Tree<T>>>
{
public:
  using observer_type = AbstractStructureObserver<Tree<T>>;
  using observed_type = Observed<observer_type>;
  constexpr static bool is_tree = true;

  Tree(Scene& scene, std::unique_ptr<T> root);
  ~Tree();

public:
  void insert(TreeOwningContext<T>& context);
  void move(TreeMoveContext<T>& context);
  void remove(TreeOwningContext<T>& context);
  bool can_move_object(const TreeMoveContext<T>& new_context) const;
  std::unique_ptr<T> replace_root(std::unique_ptr<T> new_root);
  T& root() const;

  std::set<T*> items() const override;
  size_t position(const T& item) const override;
  const T* predecessor(const T& sibling) const override;

  T& insert(std::unique_ptr<T> t) override;
  std::unique_ptr<T> remove(T& t) override;
  void invalidate() override;

private:
  std::unique_ptr<T> m_root;

  mutable bool m_item_cache_is_dirty = true;
  mutable std::set<T*> m_item_cache;
};

}  // namespace omm

