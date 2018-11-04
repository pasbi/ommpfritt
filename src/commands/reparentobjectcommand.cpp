#include "commands/reparentobjectcommand.h"
#include "objects/object.h"
#include "scene/project.h"
#include "scene/objecttreecontext.h"

namespace
{

omm::ObjectTreeContext make_old_context(const omm::ObjectTreeContext& new_context)
{
  return omm::ObjectTreeContext(new_context.subject);
}

}  // namespace



namespace omm
{

ReparentObjectCommand::ReparentObjectCommand( Project& project,
                                              const std::vector<ObjectTreeContext>& new_contextes )
  : Command(project, QObject::tr("reparent").toStdString())
  , m_old_contextes(::transform<ObjectTreeContext>(new_contextes, make_old_context))
  , m_new_contextes(new_contextes)
{
}

void ReparentObjectCommand::redo()
{
  for (const auto& context : m_new_contextes) {
    m_project.scene().move_object(context);
  }
}

void ReparentObjectCommand::undo()
{
  for (auto&& it = m_old_contextes.crbegin(); it != m_old_contextes.crend(); ++it) {
    m_project.scene().move_object(*it);
  }
}

}  // namespace omm
