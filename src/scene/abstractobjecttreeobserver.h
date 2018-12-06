#pragma once

#include "abstractraiiguard.h"

class QAbstractItemModel;

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
public:
  using item_model = QAbstractItemModel;
  using item_type = Object;
protected:
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_inserter_guard(Object& parent, int row) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard>
  acquire_mover_guard(const ObjectTreeMoveContext& context) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_remover_guard(const Object& object) = 0;

  virtual std::unique_ptr<AbstractRAIIGuard> acquire_reseter_guard() = 0;

  friend class Scene;
};

}  // namespace omm
