#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodemodel.h"
#include "common.h"

namespace omm
{

Node::Node(Scene* scene)
  : PropertyOwner(scene)
{
}

Node::~Node()
{
}

std::set<AbstractPort*> Node::ports() const
{
  return ::transform<AbstractPort*>(m_ports, [](const std::unique_ptr<AbstractPort>& p) {
    return p.get();
  });
}

void Node::set_model(NodeModel* model)
{
  m_model = model;
}

void Node::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  AbstractPropertyOwner::serialize(serializer, root);
  serializer.set_value(Vec2f(pos()), make_pointer(root, POS_PTR));

  std::vector<const InputPort*> connection_inputs;
  connection_inputs.reserve(m_ports.size());
  for (const auto& port : m_ports) {
    if (port->port_type == PortType::Input && static_cast<InputPort&>(*port).connected_output() != nullptr) {
      connection_inputs.push_back(static_cast<const InputPort*>(port.get()));
    }
  }
  connection_inputs.shrink_to_fit();
  const auto connections_ptr = make_pointer(root, CONNECTIONS_PTR);
  serializer.start_array(connection_inputs.size(), connections_ptr);
  for (std::size_t i = 0; i < connection_inputs.size(); ++i) {
    const auto iptr = make_pointer(connections_ptr, i);
    const InputPort& input = *connection_inputs.at(i);
    const OutputPort& output = *input.connected_output();
    serializer.set_value(input.index, make_pointer(iptr, INPUT_PORT_PTR));
    serializer.set_value(output.index, make_pointer(iptr, OUTPUT_PORT_PTR));
    serializer.set_value(&output.node, make_pointer(iptr, CONNECTED_NODE_PTR));
  }
  serializer.end_array();
}

void Node::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  AbstractPropertyOwner::deserialize(deserializer, root);
  m_pos = deserializer.get_vec2f(make_pointer(root, POS_PTR)).to_pointf();

  const auto connections_ptr = make_pointer(root, CONNECTIONS_PTR);
  const std::size_t n = deserializer.array_size(connections_ptr);
  for (std::size_t i = 0; i < n; ++i) {
    const auto iptr = make_pointer(connections_ptr, i);
    ConnectionIds connection_ids;
    connection_ids.input_port = deserializer.get_size_t(make_pointer(iptr, INPUT_PORT_PTR));
    connection_ids.output_port = deserializer.get_size_t(make_pointer(iptr, OUTPUT_PORT_PTR));
    connection_ids.node_id = deserializer.get_size_t(make_pointer(iptr, CONNECTED_NODE_PTR));
    m_connection_ids.push_back(connection_ids);
  }
  deserializer.register_reference_polisher(*this);
}

void Node::set_pos(const QPointF& pos)
{
  m_pos = pos;
  Q_EMIT pos_changed();
}

QPointF Node::pos() const
{
  return m_pos;
}

bool Node::is_free() const
{
  for (const auto& port : m_ports) {
    if (port->is_connected()) {
      return false;
    }
  }
  return true;
}

QString Node::name() const
{
  return QCoreApplication::translate("any-context", type().toStdString().c_str());
}

std::set<Node*> Node::successors() const
{
  std::set<Node*> successors;
  for (const auto& port : m_ports) {
    if (port->port_type == PortType::Output) {
      const OutputPort& op = static_cast<OutputPort&>(*port);
      for (const InputPort* ip : op.connected_inputs()) {
        successors.insert(&ip->node);
      }
    }
  }
  return successors;
}

AbstractPort& Node::add_port(std::unique_ptr<AbstractPort> port)
{
  auto& ref = *port;
  m_ports.insert(std::move(port));
  if (m_model != nullptr) {
    m_model->notify_topology_changed();
  }
  return ref;
}

std::unique_ptr<AbstractPort> Node::remove_port(const AbstractPort& port)
{
  for (auto it = m_ports.begin(); it != m_ports.end(); ++it) {
    if (it->get() == &port) {
      auto port = std::move(m_ports.extract(it).value());
      m_model->notify_topology_changed();
      return port;
    }
  }
  return nullptr;
}

Property& Node::add_property(const QString& key, std::unique_ptr<Property> property)
{
  auto& ref = PropertyOwner::add_property(key, std::move(property));
  add_port<PropertyPort<PortType::Input>>(ref);
  add_port<PropertyPort<PortType::Output>>(ref);
  return ref;
}

std::unique_ptr<Property> Node::extract_property(const QString& key)
{
  auto property = PropertyOwner::extract_property(key);
  if (InputPort* ip = find_port<InputPort>(*property); ip) {
    remove_port(*ip);
  }
  if (OutputPort* op = find_port<OutputPort>(*property); op) {
    remove_port(*op);
  }
  return property;
}

void Node::update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map)
{
  for (const ConnectionIds& cids : m_connection_ids) {
    Node& node = static_cast<Node&>(*map.at(cids.node_id));
    InputPort* input = find_port<InputPort>(cids.input_port);
    OutputPort* output = node.find_port<OutputPort>(cids.output_port);
    assert (input != nullptr && output != nullptr);
    input->connect(output);
  }
}

}  // namespace omm
