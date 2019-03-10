#include "commands/removecommand.h"

#include <algorithm>
#include "scene/tree.h"
#include "scene/list.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "renderers/style.h"

namespace
{

template<typename StructureT>
auto make_contextes( const StructureT& structure,
                     const std::set<typename StructureT::item_type*>& selection )
{
  using item_type = typename StructureT::item_type;
  using context_type = typename omm::Contextes<item_type>::Owning;
  std::vector<context_type> contextes;
  contextes.reserve(selection.size());
  for (auto item : selection) {
    contextes.emplace_back(*item, structure.predecessor(*item));
  }

  topological_context_sort(contextes);
  std::reverse(contextes.begin(), contextes.end());

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
  for (auto&& context : m_contextes) {
    assert(context.subject.owns());
    m_structure.insert(context);
  }
  // m_structure.selection_changed();  // TODO
}

template class RemoveCommand<ObjectTree>;
template class RemoveCommand<Tree<Object>>;
template class RemoveCommand<List<Style>>;
template class RemoveCommand<List<Tag>>;

}  // namespace omm
