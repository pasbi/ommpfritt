#pragma once

#include <memory>
#include "commands/command.h"
#include "properties/stringproperty.h"
#include "objects/object.h"
#include "renderers/style.h"

namespace omm
{

template<typename StructureT>
class AddCommand : public Command
{
public:
  using item_type = typename StructureT::item_type;
  AddCommand(StructureT& structure, std::unique_ptr<item_type> item)
    : Command("add")
    , m_item(std::move(item))
    , m_structure(structure)
  {
    static int i = 0;
    const auto name = m_item->type() + " " + std::to_string(i++);
    const auto key = AbstractPropertyOwner::NAME_PROPERTY_KEY;
    m_item->template property<StringProperty>(key).set_value(name);
  }

  void undo() override
  {
    if (m_item.owns()) {
      LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
    } else {
      m_item.capture(m_structure.remove(m_item));

      // important. else, handle or property manager might point to dangling objects
      // m_structure.selection_changed();  // TODO
    }
  }

  void redo() override
  {
    if (!m_item.owns()) {
      LOG(FATAL) << "Command cannot give away non-owned object.";
    } else {
      m_structure.insert(m_item.release());
      // m_structure.selection_changed();  // TODO
    }
  }

private:
  MaybeOwner<item_type> m_item;
  StructureT& m_structure;
};

}  // namespace omm
