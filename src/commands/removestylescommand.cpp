#include "commands/removestylescommand.h"
#include "scene/list.h"
#include "renderers/style.h"

namespace
{

// TODO same as RemoveObjectsCommand.cpp:make_contextes()
auto make_contextes(omm::List<omm::Style>& structure, const std::set<omm::Style*>& selection)
{
  std::vector<omm::StyleListOwningContext> contextes;
  contextes.reserve(selection.size());
  for (auto style : selection) {
    size_t position = structure.position(*style);
    contextes.emplace_back(*style, position == 0 ? nullptr : &structure.item(position-1));
  }

  return contextes;
}

}  // namespace

namespace omm
{

RemoveStylesCommand
::RemoveStylesCommand(List<Style>& structure, const std::set<omm::Style*>& styles)
  : Command(QObject::tr("remove").toStdString())
  , m_structure(structure), m_contextes(make_contextes(structure, styles))
{
}

void RemoveStylesCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    auto& context = *it;

    // if predecessor is not null, it must had been inserted in the object tree.
    m_structure.insert(context);
  }
  // m_structure.selection_changed();  // TODO
}

void RemoveStylesCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(!context.subject.owns());
    // assert(m_structure.find_reference_holders(context.subject).size() == 0);  // TODO
    m_structure.remove(context);
  }
  // important. else, handle or property manager might point to dangling objects
  // m_structure.selection_changed();  // TODO
}

}  // namespace omm