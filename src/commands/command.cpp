#include <glog/logging.h>

#include "scene/scene.h"

#include "commands/command.h"


namespace omm
{

Command::Command(const std::string& label)
  : QUndoCommand(QString::fromStdString(label))
{
}

std::string Command::label() const
{
  return actionText().toStdString();
}

}  // namespace omm

