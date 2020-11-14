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
  std::vector<std::unique_ptr<Property>> m_swapped_properties;
  void swap_user_properties();
};

}  // namespace omm
