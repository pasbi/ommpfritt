#include "commands/removeobjectscommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

auto make_contextes(const std::set<omm::Object*>& selection)
{
  std::vector<omm::OwningObjectTreeContext> contextes;
  contextes.reserve(selection.size());
  for (auto object : selection) {
    contextes.emplace_back(*object);
  }
  return contextes;
}

}  // namespace

namespace omm
{

RemoveObjectsCommand::RemoveObjectsCommand(Scene& scene)
  : Command(QObject::tr("remove").toStdString())
  , m_contextes(std::move(make_contextes(scene.selected_objects())))
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
  // important. else, handle or property manager might point to dangling objects
  m_scene.clear_selection();
}

void RemoveObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    m_scene.insert_object(*it);
  }
}

}  // namespace omm
