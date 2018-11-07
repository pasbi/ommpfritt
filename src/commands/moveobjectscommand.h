#pragma once

#include "commands/command.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class MoveObjectsCommand : public Command
{
public:
  MoveObjectsCommand(Scene& scene, const std::vector<MoveObjectTreeContext>& new_contextes);
  void redo();
  void undo();

private:
  const std::vector<MoveObjectTreeContext> m_new_contextes;
  const std::vector<MoveObjectTreeContext> m_old_contextes;
};

}  // namespace omm
