#include "commands/userpropertyconfigcommand.h"
#include "aspects/propertyowner.h"

namespace omm
{

UserPropertyConfigCommand::UserPropertyConfigCommand(
  AbstractPropertyOwner& owner, std::vector<std::unique_ptr<Property>> properties )
  : Command("Modify user properties")
  , m_owner(owner)
  , m_new_properties(std::move(properties))
{

}

void UserPropertyConfigCommand::undo()
{

}

void UserPropertyConfigCommand::redo()
{

}

void UserPropertyConfigCommand
::swap_user_properties(properties_type& new_properties, properties_type& old_properties)
{
  assert(old_properties.size() == 0);
  for (auto&& property : m_owner.properties().values()) {
    if (property->is_user_property()) {
      // m_owner.properties.
    }
  }
}

}  // namespace omm
