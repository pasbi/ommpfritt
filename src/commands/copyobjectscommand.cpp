#include "commands/copyobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace omm
{

CopyObjectsCommand
::CopyObjectsCommand(Scene& scene, std::vector<OwningObjectTreeContext> contextes)
  : Command(scene, QObject::tr("copy").toStdString())
  , m_contextes(std::move(contextes))
{
  std::for_each(m_contextes.begin(), m_contextes.end(), [](auto& context) {
    context.subject.capture_by_copy();
  });
}

void CopyObjectsCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(context.subject.owns());
    scene.insert_object(context);
  }
}

void CopyObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(!it->subject.owns());
    scene.remove_object(*it);
  }
}

}  // namespace omm
