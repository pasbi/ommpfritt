#include "commands/copyobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace omm
{

CopyObjectsCommand
::CopyObjectsCommand(Scene& scene, std::vector<OwningObjectTreeContext> contextes)
  : Command(QObject::tr("copy").toStdString())
  , m_contextes(std::move(contextes))
  , m_scene(scene)
{
  const Object* predecessor = m_contextes.front().predecessor;
  for (auto& context : m_contextes) {
    context.subject.capture_by_copy();
    context.predecessor = predecessor;
    predecessor = &context.subject.reference();
  }
}

void CopyObjectsCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(context.subject.owns());
    m_scene.insert_object(context);
  }
}

void CopyObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(!it->subject.owns());
    m_scene.remove_object(*it);
  }
}

}  // namespace omm
