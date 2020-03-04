#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodemodel.h"
#include "common.h"

namespace omm
{

std::map<QString, const Node::Detail*> Node::m_details;

Node::Node(NodeModel& model)
  : PropertyOwner(&model.scene())
  , m_model(model)
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
  if (pos != m_pos) {
    m_pos = pos;
    Q_EMIT pos_changed(pos);
  }
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

AbstractNodeCompiler::Language Node::language() const
{
  return model().language();
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

QString Node::title() const
{
  return QCoreApplication::translate("any-context", type().toStdString().c_str());
}

QString Node::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  Q_UNREACHABLE();
  return NodeCompilerTypes::INVALID_TYPE;
}

QString Node::input_data_type(const InputPort& port) const
{
  if (OutputPort* op = port.connected_output(); op != nullptr) {
    return op->data_type();
  } else {
    return NodeCompilerTypes::INVALID_TYPE;
  }
}

bool Node::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  // do not perform any type conversions by default.
  // If you want your node to be more flexible, override this method.
  return port.data_type() == type;
}

bool Node::is_valid() const
{
  for (const auto& p : m_ports) {
    if (p->data_type() == NodeCompilerTypes::INVALID_TYPE) {
      return false;
    }
  }
  return true;
}

std::unique_ptr<Node> Node::clone(NodeModel& model) const
{
  auto clone = Node::make(type(), model);
  copy_properties(*clone, CopiedProperties::User | CopiedProperties::Compatible);
  clone->set_pos(pos());
  return clone;
}

AbstractPort& Node::add_port(std::unique_ptr<AbstractPort> port)
{
  auto& ref = *port;
  m_ports.insert(std::move(port));
  m_model.emit_topology_changed();
  Q_EMIT ports_changed();
  return ref;
}

std::unique_ptr<AbstractPort> Node::remove_port(const AbstractPort& port)
{
  for (auto it = m_ports.begin(); it != m_ports.end(); ++it) {
    if (it->get() == &port) {
      auto port = std::move(m_ports.extract(it).value());
      Q_EMIT m_model.emit_topology_changed();
      Q_EMIT ports_changed();
      return port;
    }
  }
  return nullptr;
}

Property& Node::add_property(const QString& key, std::unique_ptr<Property> property)
{
  return add_property(key, std::move(property), PortType::Both);
}

Property& Node::add_property(const QString &key, std::unique_ptr<Property> property, PortType type)
{
  auto& ref = PropertyOwner::add_property(key, std::move(property));
  InputPort* ip = nullptr;
  OutputPort* op = nullptr;
  if (!!(type & PortType::Input)) {
    ip = &add_port<PropertyInputPort>([this, key]() { return this->property(key); });
  }
  if (!!(type & PortType::Output)) {
    op = &add_port<PropertyOutputPort>([this, key]() { return this->property(key); });
  }
  if (ip != nullptr && op != nullptr && ip->index != op->index) {
    LERROR << "indices of sibling ports are not identical.";
    LINFO << "rearrange the ports of this node.";
    abort();
  }
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
  QSignalBlocker blocker(model());
  for (const ConnectionIds& cids : m_connection_ids) {
    Node& node = static_cast<Node&>(*map.at(cids.node_id));
    assert(&node.model() == &model());
    InputPort* input = find_port<InputPort>(cids.input_port);
    OutputPort* output = node.find_port<OutputPort>(cids.output_port);
    if (input != nullptr && output != nullptr) {
      input->connect(output);
    }
  }
}

}  // namespace omm
