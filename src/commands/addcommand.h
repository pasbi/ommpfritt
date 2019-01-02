#pragma once

#include <memory>
#include "commands/command.h"
#include "properties/stringproperty.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/contextes.h"

namespace omm
{

namespace detail
{

template<typename ContextT, typename ItemT, typename StructureT>
ContextT make_context(std::unique_ptr<ItemT> item, StructureT& structure)
{
  const auto get_siblings = [&structure]() {
    if constexpr (StructureT::is_tree) {
      return structure.root().children();
    } else {
      return structure.ordered_items();
    }
  };

  const auto make_context = [&structure, &get_siblings](ItemT& item) {
    const auto siblings = get_siblings();
    auto* predecessor = siblings.size() == 0 ? nullptr : siblings.back();
    if constexpr (StructureT::is_tree) {
      return ContextT(item, structure.root(), predecessor);
    } else {
      return ContextT(item, predecessor);
    }
  };

  auto context = make_context(*item);
  context.subject.capture(std::move(item));
  return context;
}

}  // namespace detail

template<typename StructureT>
class AddCommand : public Command
{
public:
  using item_type = typename StructureT::item_type;
  using context_type = typename Contextes<item_type>::Owning;

  AddCommand(StructureT& structure, std::unique_ptr<item_type> item)
    : AddCommand(structure, detail::make_context<context_type>(std::move(item), structure))
  {
    static int i = 0;
    const auto name = m_context.subject->type() + " " + std::to_string(i++);
    m_context.subject->property(AbstractPropertyOwner::NAME_PROPERTY_KEY).set(name);
  }

  AddCommand(StructureT& structure, context_type context)
    : Command("add")
    , m_context(std::move(context))
    , m_structure(structure)
  {
  }

  void undo() override
  {
    if (m_context.subject.owns()) {
      LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
    } else {
      m_context.subject.capture(m_structure.remove(m_context.subject));

      // important. else, handle or property manager might point to dangling objects
      // m_structure.selection_changed();  // TODO
    }
  }

  void redo() override
  {
    if (!m_context.subject.owns()) {
      LOG(FATAL) << "Command cannot give away non-owned object.";
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
