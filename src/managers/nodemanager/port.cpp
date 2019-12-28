#include "managers/nodemanager/port.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/node.h"

namespace omm
{

InputPort::InputPort(Node& node, std::size_t index)
  : InputPort(PortFlavor::Ordinary, node, index)
{
}

InputPort::InputPort(PortFlavor flavor, Node& node, std::size_t index)
  : Port<PortType::Input>(flavor, node, index)
{
}

InputPort::InputPort(const InputPort& other, Node& node)
  : Port<PortType::Input>(other, node)
{
}

bool InputPort::is_connected(const AbstractPort* other) const
{
  if (other->port_type == PortType::Input) {
    return false;
  } else {
    return m_connected_output == other;
  }
}

bool InputPort::is_connected() const
{
  return m_connected_output != nullptr;
}

void InputPort::connect(OutputPort* port)
{
  if (m_connected_output != port) {
    if (m_connected_output != nullptr) {
      m_connected_output->disconnect(this, Tag());
    } else if (port != nullptr) {
      port->connect(this, Tag());
    }
    m_connected_output = port;
    node.model()->notify_topology_changed();
  }
}

OutputPort::OutputPort(PortFlavor flavor, Node& node, std::size_t index)
  : Port<PortType::Output>(flavor, node, index)
{

}

OutputPort::OutputPort(Node& node, std::size_t index)
  : OutputPort(PortFlavor::Ordinary, node, index)
{
}

OutputPort::OutputPort(const OutputPort& other, Node& node) : Port<PortType::Output>(other, node)
{
}

bool OutputPort::is_connected(const AbstractPort* other) const
{
  if (other->port_type == PortType::Input) {
    return static_cast<const InputPort*>(other)->is_connected(this);
  } else {
    return false;
  }
}

AbstractPort::AbstractPort(PortFlavor flavor, PortType port_type, Node& node, std::size_t index)
  : port_type(port_type), flavor(flavor), node(node), index(index)
{
}

AbstractPort::AbstractPort(const AbstractPort& other, Node& node)
  : port_type(other.port_type), flavor(other.flavor), node(node), index(other.index)
{

}

AbstractPort::~AbstractPort()
{
}

bool AbstractPort::is_connected(const AbstractPort* other) const
{
  return visit(*this, [other](const auto& port) {
    return port.is_connected(other);
  });
}

bool AbstractPort::is_connected() const
{
  return visit(*this, [](const auto& port) {
    return port.is_connected();
  });
}

bool OutputPort::is_connected() const
{
  return !m_connections.empty();
}

void OutputPort::disconnect(InputPort* port, InputPort::Tag)
{
  m_connections.erase(port);
}

void OutputPort::connect(InputPort* port, InputPort::Tag)
{
  m_connections.insert(port);
}

}  // namespace omm
