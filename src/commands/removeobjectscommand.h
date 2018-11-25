#pragma once

#include "commands/command.h"
#include "common.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class RemoveObjectsCommand : public Command
{
public:
  RemoveObjectsCommand(Scene& scene);

  void undo() override;
  void redo() override;

private:
  std::vector<OwningObjectTreeContext> m_contextes;
  Scene& m_scene;
};

}  // namespace
