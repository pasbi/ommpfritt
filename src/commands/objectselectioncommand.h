#pragma once

#include "commands/command.h"
#include <set>

namespace omm
{
class Object;

class ObjectSelectionCommand : public Command
{
public:
  explicit ObjectSelectionCommand(Scene& scene, const std::set<Object*>& new_object_selection);
  void undo() override;
  void redo() override;

private:
  Scene& m_scene;
  const std::set<Object*> m_new_object_selection;
  const std::set<Object*> m_old_object_selection;
};

}  // namespace omm
