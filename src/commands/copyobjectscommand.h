#pragma once

#include "commands/command.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class CopyObjectsCommand : public Command
{
public:
  CopyObjectsCommand(Scene& scene, std::vector<OwningObjectTreeContext> contextes);

  void undo() override;
  void redo() override;

private:
  std::vector<OwningObjectTreeContext> m_contextes;
  Scene& m_scene;
};

}  // namespace
