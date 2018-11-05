#include "commands/copyobjectscommand.h"
#include "objects/object.h"
#include "scene/project.h"
#include "scene/objecttreecontext.h"

namespace omm
{

CopyObjectsCommand
::CopyObjectsCommand(Project& project, std::vector<CopyObjectTreeContext>&& contextes)
  : Command(project, QObject::tr("copy").toStdString())
  , m_contextes(std::move(contextes))
{
}

void CopyObjectsCommand::redo()
{
  for (auto&& context : m_contextes) {
    m_project.scene().insert_object(context);
  }
}

void CopyObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    m_project.scene().remove_object(*it);
  }
}

}  // namespace omm