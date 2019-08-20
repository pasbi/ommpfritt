#pragma once

#include <memory>
#include "abstractraiiguard.h"
// #include "scene/list.h"
// #include "scene/tree.h"

namespace omm
{

template<typename> class AbstractStructureObserver;

template<typename> class Tree;
template<typename> class TreeMoveContext;

template<typename T> class AbstractStructureObserver<Tree<T>>
{
  friend class Tree<T>;
};

template<typename> class List;
template<typename> class ListMoveContext;

template<typename T> class AbstractStructureObserver<List<T>>
{
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(int) { return nullptr; }
  virtual std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const ListMoveContext<T>&) { return nullptr; }
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(int) { return nullptr; }
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() { return nullptr; }
  friend class List<T>;
};

}  // namespace omm
