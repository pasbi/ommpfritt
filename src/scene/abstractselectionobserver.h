#pragma once

#include <set>
#include "aspects/propertyowner.h"

namespace omm
{

class Object;
class Style;
class Tag;
class Tool;
class Scene;

class AbstractSelectionObserver
{
protected:
  virtual void on_selection_changed(const std::set<AbstractPropertyOwner*>& selection);
  virtual void on_object_selection_changed(const std::set<Object*>& selection);
  virtual void on_style_selection_changed(const std::set<Style*>& selection);
  virtual void on_tag_selection_changed(const std::set<Tag*>& selection);
  virtual void on_tool_selection_changed(const std::set<Tool*>& selection);
  friend class Scene;

  virtual void on_selection_changed( const std::set<AbstractPropertyOwner*>& selection,
                                     AbstractPropertyOwner::Kind kind );
};

}  // namespace omm
