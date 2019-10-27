#include "scene/scene.h"

#include "commands/command.h"


namespace omm
{

Command::Command(const QString& label) : QUndoCommand(label) { }
QString Command::label() const { return actionText(); }

}  // namespace omm

