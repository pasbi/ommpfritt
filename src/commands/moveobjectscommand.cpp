#include "commands/moveobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

omm::MoveObjectTreeContext make_old_context(const omm::MoveObjectTreeContext& new_context)
{
  return omm::MoveObjectTreeContext(new_context.subject);
}

}  // namespace



namespace omm
{

MoveObjectsCommand
::MoveObjectsCommand(Scene& scene, const std::vector<MoveObjectTreeContext>& new_contextes)
  : Command(QObject::tr("reparent").toStdString())
  , m_old_contextes(::transform<MoveObjectTreeContext>(new_contextes, make_old_context))
  , m_new_contextes(new_contextes)
  , m_scene(scene)
{
}

void MoveObjectsCommand::redo()
{
  for (const auto& context : m_new_contextes) {
    m_scene.move_object(context);
  }
}

void MoveObjectsCommand::undo()
{
  for (auto&& it = m_old_contextes.crbegin(); it != m_old_contextes.crend(); ++it) {
    m_scene.move_object(*it);
  }
}

}  // namespace omm
