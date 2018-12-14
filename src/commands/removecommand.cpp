#include "commands/removecommand.h"

#include <algorithm>
#include "scene/tree.h"
#include "scene/list.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "renderers/style.h"

namespace
{

template<typename iterator, typename pred>
iterator move_if(iterator begin, iterator end, const pred& predicate)
{
  const iterator it = std::find_if(begin, end, predicate);
  if (it != end) {
    std::iter_swap(it, std::prev(end));
    return std::prev(end);
  } else {
    return it;
  }
}

template<typename T>
void stupid_topological_sort(std::vector<T>& ts)
{
  // Stupid because I guess it's O(n^2). It's possible to do it in O(n).
  const auto begin_ts = ts.begin();
  auto end_ts = ts.end();

  while (begin_ts != end_ts) {
    end_ts = move_if(begin_ts, end_ts, [&begin_ts, &end_ts](const auto& t) {
      return end_ts == std::find_if(begin_ts, end_ts, [&t](const auto& s) {
        return t.predecessor == &s.subject.get();
      });
    });

    while (true) {
      const auto current = move_if(begin_ts, end_ts, [&end_ts](const auto& t) {
        return &end_ts->subject.get() == t.predecessor;
      });
      if (current == end_ts) {
        break;
      } else {
        end_ts = current;
      }
    }
  }
}

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

  stupid_topological_sort(contextes);
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
    // if predecessor is not null, it must had been inserted in the object tree.
    assert(context.subject.owns());
    m_structure.insert(context);
  }
  // m_structure.selection_changed();  // TODO
}

template class RemoveCommand<Tree<Object>>;
template class RemoveCommand<List<Style>>;
template class RemoveCommand<List<Tag>>;

}  // namespace omm
