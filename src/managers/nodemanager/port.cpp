#include "managers/nodemanager/port.h"
#include "nodemodel.h"
#include "managers/nodemanager/node.h"

namespace omm
{

InputPort::InputPort(Node& node, std::size_t index, const QString& name)
  : Port(true, node, index, name)
{
}

bool omm::InputPort::is_connected(const Port* other) const
{
  if (other->is_input) {
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
    node.model()->notify_appearance_changed();
  }
}

OutputPort::OutputPort(Node& node, std::size_t index, const QString& name)
  : Port(false, node, index, name)
{
}

bool OutputPort::is_connected(const Port* other) const
{
  if (other->is_input) {
    return static_cast<const InputPort*>(other)->is_connected(this);
  } else {
    return false;
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

Port::Port(bool is_input, Node& node, std::size_t index, const QString& name)
  : name(name)
  , node(node)
  , index(index)
  , is_input(is_input)
{
}

Port::~Port()
{

}

}  // namespace omm
