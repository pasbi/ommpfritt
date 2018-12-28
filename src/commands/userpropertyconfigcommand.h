#pragma once

#include "commands/command.h"
#include "maybeowner.h"
#include "properties/property.h"

namespace omm
{

class AbstractPropertyOwner;
class UserPropertyConfigCommand : public Command
{
public:
  UserPropertyConfigCommand(AbstractPropertyOwner& owner,
                            std::vector<std::unique_ptr<Property>> properties);
  void undo() override;
  void redo() override;

private:
  AbstractPropertyOwner& m_owner;
  using properties_type = std::vector<std::unique_ptr<Property>>;
  properties_type m_new_properties;
  properties_type m_old_properties;
  void swap_user_properties(properties_type& new_properties, properties_type& old_properties);
};

}  // namespace omm
