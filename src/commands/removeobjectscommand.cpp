#include "commands/removeobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

auto make_contextes(const omm::ObjectRefs objects)
{
  std::vector<omm::OwningObjectTreeContext> contextes;
  contextes.reserve(objects.size());
  for (omm::Object& object : objects) {
    assert(!object.is_root());
    contextes.emplace_back(object);
  }
  return contextes;
}

}  // namespace

namespace omm
{

RemoveObjectsCommand
::RemoveObjectsCommand(Scene& scene, const ObjectRefs& objects)
  : Command(QObject::tr("remove").toStdString())
  , m_contextes(std::move(make_contextes(objects)))
  , m_scene(scene)
{
}

void RemoveObjectsCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(!context.subject.owns());
    assert(!context.subject.reference().is_root());
    m_scene.remove_object(context);
  }
}

void RemoveObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    m_scene.insert_object(*it);
  }
}

}  // namespace omm
