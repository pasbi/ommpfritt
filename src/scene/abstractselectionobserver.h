#pragma once

#include <set>

namespace omm
{

class PropertyOwner;
class Scene;

class AbstractSelectionObserver
{
protected:
  virtual void set_selection(const std::set<PropertyOwner*>& selection) = 0;
  friend class Scene;
};

}  // namespace omm
