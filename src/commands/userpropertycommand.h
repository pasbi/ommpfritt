#pragma once

#include "commands/command.h"
#include <vector>
#include <memory>
#include "properties/property.h"

namespace omm
{

class AbstractPropertyOwner;

class UserPropertyCommand : public Command
{
public:
  using pmap = std::map<QString, std::unique_ptr<Property>>;
  using cmap = std::map<QString, Property::Configuration>;

  UserPropertyCommand(const std::vector<QString>& deletions,
                      std::vector<std::pair<QString, std::unique_ptr<Property>>> additions,
                      const std::map<Property*, Property::Configuration>& changes,
                      AbstractPropertyOwner& owner);
  void undo() override { swap(); }
  void redo() override { swap(); }

private:
  std::vector<QString> m_deletions;
  std::vector<std::pair<QString, std::unique_ptr<Property>>> m_additions;
  std::map<Property*, Property::Configuration> m_changes;
  AbstractPropertyOwner& m_owner;

  void swap();
};

}  // namespace omm
