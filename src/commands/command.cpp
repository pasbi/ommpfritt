#include <glog/logging.h>

#include "scene/scene.h"

#include "commands/command.h"


namespace omm
{

Command::Command(const std::string& label)
  : QUndoCommand(QString::fromStdString(label))
{
}

}  // namespace omm

