#include "commands/removeobjectscommand.h"

#include <algorithm>
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

auto make_contextes(const std::set<omm::Object*>& selection)
{
  std::vector<omm::ObjectTreeOwningContext> contextes;
  contextes.reserve(selection.size());
  for (auto object : selection) {
    contextes.emplace_back(*object);
  }

  // assert that to-be-inserted objects' predecessor is already in the tree,
  // i.e., insert the predecessor first.
  std::sort(contextes.begin(), contextes.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.predecessor == &rhs.get_subject()) {
      return true;
    } else {
      return false;
    }
  });
  return contextes;
}

}  // namespace

namespace omm
{

RemoveObjectsCommand::RemoveObjectsCommand(Scene& scene, const std::set<omm::Object*>& objects)
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
    assert(m_scene.find_reference_holders(context.subject).size() == 0);
    m_scene.remove(context);
  }
  // important. else, handle or property manager might point to dangling objects
  m_scene.selection_changed();
}

void RemoveObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    ObjectTreeOwningContext& context = *it;

    // if predecessor is not null, it must had been inserted in the object tree.
    assert(context.predecessor == nullptr || !context.predecessor->is_root());
    m_scene.insert(context);
  }
  m_scene.selection_changed();
}

}  // namespace omm
