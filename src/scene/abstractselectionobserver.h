#pragma once

#include <set>

namespace omm
{

class AbstractPropertyOwner;
class Scene;

class AbstractSelectionObserver
{
protected:
  virtual void set_selection(const std::set<AbstractPropertyOwner*>& selection) = 0;
  friend class Scene;
};

}  // namespace omm
