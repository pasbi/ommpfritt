#pragma once

#include "commands/command.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include <memory>

namespace omm
{
template<typename StructureT> class AddCommand : public Command
{
public:
  using item_type = typename StructureT::item_type;
  using context_type = typename Contextes<item_type>::Owning;

  AddCommand(StructureT& structure, std::unique_ptr<item_type> item)
      : AddCommand(structure, context_type(std::move(item), structure))
  {
    static int i = 0;
    const auto name = m_context.subject->type() + QString(" %1").arg(i++);
    m_context.subject->property(AbstractPropertyOwner::NAME_PROPERTY_KEY)->set(name);
  }

  AddCommand(StructureT& structure, context_type context)
      : Command(QObject::tr("add")), m_context(std::move(context)), m_structure(structure)
  {
  }

  void undo() override
  {
    if (m_context.subject.owns()) {
      LFATAL("Command already owns object. Obtaining ownership again is absurd.");
    } else {
      m_context.subject.capture(m_structure.remove(m_context.subject));

      // important. else, handle or property manager might point to dangling objects
      // m_structure.selection_changed();  // TODO
    }
  }

  void redo() override
  {
    if (!m_context.subject.owns()) {
      LFATAL("Command cannot give away non-owned object.");
    } else {
      m_structure.insert(m_context);
      // m_structure.selection_changed();  // TODO
    }
  }

private:
  context_type m_context;
  StructureT& m_structure;
};

}  // namespace omm
