#include "scene/scene.h"

#include "commands/command.h"

namespace omm
{
Command::Command(const QString& label) : QUndoCommand(label)
{
}

bool Command::is_noop() const
{
  return false;
}

}  // namespace omm
