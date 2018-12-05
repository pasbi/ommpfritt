#pragma once

#include "abstractraiiguard.h"

namespace omm
{

class Scene;

class AbstractStyleListObserver
{
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(int row) = 0;
  // virtual std::unique_ptr<AbstractMoverGuard> acquire_mover_guard() = 0;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(int row) = 0;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() = 0;
  friend class Scene;
};

}  // namespace omm
