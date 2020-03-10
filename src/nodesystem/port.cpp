#include "nodesystem/port.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/node.h"

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

bool InputPort::accepts_data_type(const QString& type) const
{
  return node.accepts_input_data_type(type, *this);
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
    node.model().emit_topology_changed();
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

QString AbstractPort::uuid() const
{
  std::stringstream ss;
  ss << static_cast<const void*>(this);
  return "p" + QString::fromStdString(ss.str());
}

std::set<AbstractPort*> AbstractPort::connected_ports() const
{
  switch (port_type) {
  case PortType::Input:
    if (AbstractPort* op = static_cast<const InputPort*>(this)->connected_output(); op != nullptr) {
      return { op };
    } else {
      return {};
    }
  case PortType::Output:
    return ::transform<AbstractPort*>(static_cast<const OutputPort*>(this)->connected_inputs());
  default:
    Q_UNREACHABLE();
    return {};
  }
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
