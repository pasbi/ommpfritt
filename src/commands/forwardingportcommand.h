#pragma once

#include "commands/command.h"
#include "nodesystem/port.h"

namespace omm
{

namespace nodes
{
class ReferenceNode;
}  // namespace nodes;

class ForwardingPortCommand : public Command
{
protected:
  ForwardingPortCommand(const QString& label,
                        nodes::ReferenceNode& node,
                        nodes::PortType port_type,
                        const QString& key);
  void add();
  void remove();

private:
  nodes::ReferenceNode& m_node;
  const nodes::PortType m_port_type;
  const QString m_key;
};

class RemoveForwardingPortCommand : public ForwardingPortCommand
{
public:
  RemoveForwardingPortCommand(nodes::ReferenceNode& node, nodes::PortType port_type, const QString& key);
  void undo() override;
  void redo() override;
};

class AddForwardingPortCommand : public ForwardingPortCommand
{
public:
  AddForwardingPortCommand(nodes::ReferenceNode& node, nodes::PortType port_type, const QString& key);
  void undo() override;
  void redo() override;
};

}  // namespace omm
