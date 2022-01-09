#pragma once

#include "commands/command.h"
#include "properties/property.h"
#include <memory>
#include <vector>

namespace omm
{

class DisconnectPortsCommand;
class ConnectPortsCommand;
class AbstractPropertyOwner;

namespace nodes
{
class NodeModel;
}  // namespace nodes

class UserPropertyCommand : public Command
{
public:
  using pmap = std::map<QString, std::unique_ptr<Property>>;
  using cmap = std::map<QString, PropertyConfiguration>;

  UserPropertyCommand(const std::vector<QString>& deletions,
                      std::vector<std::pair<QString, std::unique_ptr<Property>>> additions,
                      const std::map<Property*, PropertyConfiguration>& changes,
                      AbstractPropertyOwner& owner);
  ~UserPropertyCommand() override;
  UserPropertyCommand(UserPropertyCommand&&) = delete;
  UserPropertyCommand(const UserPropertyCommand&) = delete;
  UserPropertyCommand& operator=(UserPropertyCommand&&) = delete;
  UserPropertyCommand& operator=(const UserPropertyCommand&) = delete;
  void undo() override;
  void redo() override;

private:
  using Properties = std::vector<std::pair<QString, std::unique_ptr<Property>>>;
  Properties m_deletions;
  Properties m_additions;
  using Configurations = std::map<Property*, PropertyConfiguration>;
  Configurations m_changes;

  AbstractPropertyOwner& m_owner;
  std::set<std::unique_ptr<DisconnectPortsCommand>> m_broken_connections;

  void extract(Properties& ps);
  void insert(Properties& ps);
  static void change(Configurations& configurations);

  void swap();
  nodes::NodeModel* m_node_model = nullptr;
};

}  // namespace omm
