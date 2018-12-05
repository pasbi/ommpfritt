#pragma once

#include "commands/command.h"
#include "scene/contextes.h"

namespace omm
{

class CopyObjectsCommand : public Command
{
public:
  CopyObjectsCommand(Scene& scene, std::vector<ObjectTreeOwningContext> contextes);

  void undo() override;
  void redo() override;

private:
  std::vector<ObjectTreeOwningContext> m_contextes;
  Scene& m_scene;
};

}  // namespace
