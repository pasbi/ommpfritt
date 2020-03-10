#pragma once

#include "nodesystem/port.h"
#include "commands/command.h"

namespace omm
{

class ReferenceNode;
class ForwardingPortCommand : public Command
{
protected:
  ForwardingPortCommand(const QString& label, ReferenceNode& node,
                        PortType port_type, const QString& key);
  void add();
  void remove();

private:
  ReferenceNode& m_node;
  const PortType m_port_type;
  const QString m_key;
};

class RemoveForwardingPortCommand : public ForwardingPortCommand
{
public:
  RemoveForwardingPortCommand(ReferenceNode& node, PortType port_type, const QString& key);
  void undo() override { add(); }
  void redo() override { remove(); }
};

class AddForwardingPortCommand : public ForwardingPortCommand
{
public:
  AddForwardingPortCommand(ReferenceNode& node, PortType port_type, const QString& key);
  void undo() override { remove(); }
  void redo() override { add(); }
};

}  // namespace omm
