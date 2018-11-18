#pragma once

#include <set>

namespace omm
{

class HasProperties;
class Scene;

class AbstractSelectionObserver
{
protected:
  virtual void set_selection(const std::set<HasProperties*>& selection) = 0;
  friend class Scene;
};

}  // namespace omm
