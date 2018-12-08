#include "commands/removecommand.h"

#include <algorithm>
#include "scene/tree.h"
#include "scene/list.h"
#include "objects/object.h"
#include "renderers/style.h"

namespace
{

template<typename StructureT>
auto make_contextes( const StructureT& structure,
                     const std::set<typename StructureT::item_type*>& selection )
{
  std::vector<typename omm::Contextes<typename StructureT::item_type>::Owning> contextes;
  contextes.reserve(selection.size());
  for (auto item : selection) {
    contextes.emplace_back(*item, structure.predecessor(*item));
  }

  return contextes;
}

}  // namespace

namespace omm
{

template<typename StructureT>
RemoveCommand<StructureT>::RemoveCommand(StructureT& structure, const std::set<item_type*>& items)
  : Command(QObject::tr("remove").toStdString())
  , m_contextes(std::move(make_contextes(structure, items)))
  , m_structure(structure)
{
}

template<typename StructureT> void RemoveCommand<StructureT>::redo()
{
  for (auto&& context : m_contextes) {
    assert(!context.subject.owns());
    // assert(!context.subject.reference().is_root()); // TODO
    // assert(m_structure.find_reference_holders(context.subject).size() == 0);  // TODO
    m_structure.remove(context);
  }
  // important. else, handle or property manager might point to dangling objects
  // m_structure.selection_changed();  // TODO
}

template<typename StructureT> void RemoveCommand<StructureT>::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    auto& context = *it;

    // if predecessor is not null, it must had been inserted in the object tree.
    // assert(context.predecessor == nullptr || !context.predecessor->is_root()); // TODO
    m_structure.insert(context);
  }
  // m_structure.selection_changed();  // TODO
}

template class RemoveCommand<Tree<Object>>;
template class RemoveCommand<List<Style>>;

}  // namespace omm
