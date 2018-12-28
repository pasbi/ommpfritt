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
  swap_user_properties(m_old_properties, m_new_properties);
}

void UserPropertyConfigCommand::redo()
{
  swap_user_properties(m_new_properties, m_old_properties);
}

void UserPropertyConfigCommand
::swap_user_properties(properties_type& new_properties, properties_type& old_properties)
{
  assert(old_properties.size() == 0);
  for (auto&& key : m_owner.properties().keys()) {
    if (m_owner.property(key).is_user_property()) {
      old_properties.push_back(m_owner.extract_property(key));
    }
  }

  for (auto&& property : new_properties) {
    m_owner.add_property(property->label(), std::move(property));
  }
  new_properties.clear();
}

}  // namespace omm
