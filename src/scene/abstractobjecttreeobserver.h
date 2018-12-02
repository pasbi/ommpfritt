#pragma once

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
protected:
  virtual void beginInsertObject(Object& parent, int row) = 0;
  virtual void beginInsertObject(const OwningObjectTreeContext& context) = 0;
  virtual void endInsertObject() = 0;
  virtual void beginMoveObject(const MoveObjectTreeContext& new_context) = 0;
  virtual void endMoveObject() = 0;
  virtual void beginRemoveObject(const Object& object) = 0;
  virtual void endRemoveObject() = 0;
  virtual void beginResetObjects() = 0;
  virtual void endResetObjects() = 0;
  friend class Scene;
};

}  // namespace omm
