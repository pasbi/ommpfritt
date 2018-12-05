#include "commands/moveobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

omm::ObjectTreeMoveContext make_old_context(const omm::ObjectTreeMoveContext& new_context)
{
  return omm::ObjectTreeMoveContext(new_context.subject);
}

}  // namespace



namespace omm
{

MoveObjectsCommand
::MoveObjectsCommand(Scene& scene, const std::vector<ObjectTreeMoveContext>& new_contextes)
  : Command(QObject::tr("reparent").toStdString())
  , m_old_contextes(::transform<ObjectTreeMoveContext>(new_contextes, make_old_context))
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
  for (const auto& context : m_old_contextes) {
    m_scene.move_object(context);
  }
}

}  // namespace omm
