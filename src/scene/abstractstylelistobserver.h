#pragma once

namespace omm
{

class Scene;

class AbstractStyleListObserver
{
protected:
  virtual void beginInsertStyles(int row) = 0;
  virtual void endInsertStyles() = 0;
  virtual void beginResetStyles() = 0;
  virtual void endResetStyles() = 0;
  friend class Scene;
};

}  // namespace omm
