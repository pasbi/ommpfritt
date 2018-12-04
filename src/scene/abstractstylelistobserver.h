#pragma once

namespace omm
{

class Scene;

class AbstractStyleListObserver
{
protected:
  virtual void beginInsertStyle(int row) = 0;
  virtual void endInsertStyle() = 0;
  virtual void beginResetStyles() = 0;
  virtual void endResetStyles() = 0;
  virtual void beginRemoveStyle(int row) = 0;
  virtual void endRemoveStyle() = 0;
  friend class Scene;
};

}  // namespace omm
