#pragma once

#include "commands/command.h"
#include "common.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class RemoveObjectsCommand : public Command
{
public:
  RemoveObjectsCommand(Project& project, const ObjectRefs& objects);

  void undo() override;
  void redo() override;

private:
  std::vector<OwningObjectTreeContext> m_contextes;
};

}  // namespace
