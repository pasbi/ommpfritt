#pragma once

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
public:
  struct AbstractInserterGuard { virtual ~AbstractInserterGuard() = default; };
  struct AbstractMoverGuard { virtual ~AbstractMoverGuard() = default; };
  struct AbstractRemoverGuard { virtual ~AbstractRemoverGuard() = default; };
  struct AbstractReseterGuard { virtual ~AbstractReseterGuard() = default; };


protected:
  virtual std::unique_ptr<AbstractInserterGuard>
  acquire_inserter_guard(Object& parent, int row) = 0;

  virtual std::unique_ptr<AbstractMoverGuard>
  acquire_mover_guard(const MoveObjectTreeContext& context) = 0;

  virtual std::unique_ptr<AbstractRemoverGuard> acquire_remover_guard(const Object& object) = 0;

  virtual std::unique_ptr<AbstractReseterGuard> acquire_reseter_guard() = 0;

  friend class Scene;
};

}  // namespace omm
