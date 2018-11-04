#pragma once

#include "commands/command.h"
#include "common.h"
#include "scene/scene.h"

namespace omm
{

class ReparentObjectCommand : public Command
{
public:
  explicit ReparentObjectCommand( Project& project,
                                  const std::vector<ObjectTreeContext>& new_contextes );
  void redo();
  void undo();

private:
  const std::vector<ObjectTreeContext> m_new_contextes;
  const std::vector<ObjectTreeContext> m_old_contextes;
};

}  // namespace omm
