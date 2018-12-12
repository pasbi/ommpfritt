#pragma once

#include <set>

namespace omm
{

class AbstractPropertyOwner;
class Scene;

class AbstractSelectionObserver
{
protected:

  // TODO remove the observer class or make the following method pure virtual.
  virtual void set_selection(const std::set<AbstractPropertyOwner*>& selection) {};
  friend class Scene;
};

}  // namespace omm
