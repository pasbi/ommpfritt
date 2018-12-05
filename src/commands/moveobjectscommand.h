#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{

class MoveObjectsCommand : public Command
{
public:
  MoveObjectsCommand(Scene& scene, const std::vector<ObjectTreeMoveContext>& new_contextes);
  void redo();
  void undo();

private:
  const std::vector<ObjectTreeMoveContext> m_new_contextes;
  const std::vector<ObjectTreeMoveContext> m_old_contextes;
  Scene& m_scene;
};

}  // namespace omm
