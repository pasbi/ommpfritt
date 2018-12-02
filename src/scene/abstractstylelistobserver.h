#pragma once

namespace omm
{

class Scene;

class AbstractStyleListObserver
{
protected:
  virtual void beginInsertObject(int row) = 0;
  virtual void endInsertObject() = 0;
  // virtual void beginMoveObject(const MoveObjectTreeContext& new_context) = 0;
  // virtual void endMoveObject() = 0;
  // virtual void beginRemoveObject(const Object& object) = 0;
  // virtual void endRemoveObject() = 0;
  friend class StylePool;
};

}  // namespace omm
