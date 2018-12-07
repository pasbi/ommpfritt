#include "commands/removeobjectscommand.h"

#include <algorithm>
#include "objects/object.h"
#include "scene/tree.h"

namespace
{

auto make_contextes( const omm::Tree<omm::Object>& structure,
                     const std::set<omm::Object*>& selection )
{
  std::vector<omm::ObjectTreeOwningContext> contextes;
  contextes.reserve(selection.size());
  for (auto object : selection) {
    contextes.emplace_back(*object, structure.predecessor(*object));
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

RemoveObjectsCommand
::RemoveObjectsCommand(Tree<Object>& structure, const std::set<omm::Object*>& objects)
  : Command(QObject::tr("remove").toStdString())
  , m_contextes(std::move(make_contextes(structure, objects)))
  , m_structure(structure)
{
}

void RemoveObjectsCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(!context.subject.owns());
    assert(!context.subject.reference().is_root());
    // assert(m_structure.find_reference_holders(context.subject).size() == 0);  // TODO
    m_structure.remove(context);
  }
  // important. else, handle or property manager might point to dangling objects
  // m_structure.selection_changed();  // TODO
}

void RemoveObjectsCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    ObjectTreeOwningContext& context = *it;

    // if predecessor is not null, it must had been inserted in the object tree.
    assert(context.predecessor == nullptr || !context.predecessor->is_root());
    m_structure.insert(context);
  }
  // m_structure.selection_changed();  // TODO
}

}  // namespace omm
