#include "commands/command.h"
#include "scene/project.h"

namespace omm
{

Command::Command(Project& project, const QString& text)
  : QUndoCommand(text)
  , m_project(project)
{

}

Scene& Command::scene()
{
  return m_project.scene();
}

}  // namespace omm

