#pragma once

#include "abstractraiiguard.h"

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(Object& parent, int row) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const MoveObjectTreeContext& context) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(const Object& object) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() = 0;

  friend class Scene;
};

}  // namespace omm
