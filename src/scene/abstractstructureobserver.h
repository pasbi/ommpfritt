#pragma once

#include "abstractraiiguard.h"
// #include "scene/list.h"
// #include "scene/tree.h"

namespace omm
{

template<typename> class Tree;
template<typename> class List;
template<typename> class TreeMoveContext;
template<typename> class AbstractStructureObserver;

template<typename T> class AbstractStructureObserver<Tree<T>>
{
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(T& parent, int row) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const TreeMoveContext<T>& context) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(const T& object) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() = 0;

  friend class Tree<T>;
};

template<typename T> class AbstractStructureObserver<List<T>>
{
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(int row) = 0;
  // virtual std::unique_ptr<AbstractMoverGuard> acquire_mover_guard() = 0;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(int row) = 0;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() = 0;
  friend class List<T>;
};

}  // namespace omm
