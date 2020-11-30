#include "commands/userpropertyconfigcommand.h"
#include "aspects/propertyowner.h"

namespace omm
{
UserPropertyConfigCommand::UserPropertyConfigCommand(
    AbstractPropertyOwner& owner,
    std::vector<std::unique_ptr<Property>> properties)
    : Command(QObject::tr("Modify user properties")), m_owner(owner),
      m_swapped_properties(std::move(properties))
{
}

void UserPropertyConfigCommand::undo()
{
  swap_user_properties();
}

void UserPropertyConfigCommand::redo()
{
  swap_user_properties();
}

void UserPropertyConfigCommand::swap_user_properties()
{
  decltype(m_swapped_properties) new_properties;
  std::swap(new_properties, m_swapped_properties);

  const auto keys = m_owner.properties().keys();
  for (auto&& key : keys) {
    if (m_owner.property(key)->is_user_property()) {
      m_swapped_properties.push_back(m_owner.extract_property(key));
    }
  }

  for (auto&& property : new_properties) {
    const auto label = property->label();
    auto unique_label = label;
    int i = 0;
    while (m_owner.has_property(unique_label)) {
      i += 1;
      unique_label = label + "." + QString("%1").arg(i);
    }
    property->revise();
    m_owner.add_property(unique_label, std::move(property));
  }
}

}  // namespace omm
