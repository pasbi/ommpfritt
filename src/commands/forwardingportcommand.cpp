#include "commands/forwardingportcommand.h"
#include "nodesystem/nodes/referencenode.h"

namespace omm
{

RemoveForwardingPortCommand ::RemoveForwardingPortCommand(nodes::ReferenceNode& node,
                                                          nodes::PortType port_type,
                                                          const QString& key)
    : ForwardingPortCommand(QObject::tr("Remove Forwarding Port"), node, port_type, key)
{
}

void RemoveForwardingPortCommand::undo()
{
  add();
}

void RemoveForwardingPortCommand::redo()
{
  remove();
}

AddForwardingPortCommand ::AddForwardingPortCommand(nodes::ReferenceNode& node,
                                                    nodes::PortType port_type,
                                                    const QString& key)
    : ForwardingPortCommand(QObject::tr("Add Forwarding Port"), node, port_type, key)
{
}

void AddForwardingPortCommand::undo()
{
  remove();
}

void AddForwardingPortCommand::redo()
{
  add();
}

ForwardingPortCommand ::ForwardingPortCommand(const QString& label,
                                              nodes::ReferenceNode& node,
                                              nodes::PortType port_type,
                                              const QString& key)
    : Command(label), m_node(node), m_port_type(port_type), m_key(key)
{
}

void ForwardingPortCommand::add()
{
  m_node.add_forwarding_port(m_port_type, m_key);
}

void ForwardingPortCommand::remove()
{
  m_node.remove_forwarding_port(m_port_type, m_key);
}

}  // namespace omm
