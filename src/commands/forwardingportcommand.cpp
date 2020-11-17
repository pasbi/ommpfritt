#include "commands/forwardingportcommand.h"
#include "nodesystem/nodes/referencenode.h"

namespace omm
{
RemoveForwardingPortCommand ::RemoveForwardingPortCommand(ReferenceNode& node,
                                                          PortType port_type,
                                                          const QString& key)
    : ForwardingPortCommand(QObject::tr("Remove Forwarding Port"), node, port_type, key)
{
}

AddForwardingPortCommand ::AddForwardingPortCommand(ReferenceNode& node,
                                                    PortType port_type,
                                                    const QString& key)
    : ForwardingPortCommand(QObject::tr("Add Forwarding Port"), node, port_type, key)
{
}

ForwardingPortCommand ::ForwardingPortCommand(const QString& label,
                                              ReferenceNode& node,
                                              PortType port_type,
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
