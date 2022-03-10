#include "nodesystem/node.h"
#include "common.h"
#include "nodesystem/nodemodel.h"
#include "serializers/abstractdeserializer.h"
#include <QApplication>

namespace
{

struct ConnectionIds {
  std::size_t input_port;
  std::size_t output_port;
  std::size_t node_id;
};

}  // namespace

namespace omm::nodes
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::map<QString, const Node::Detail*> Node::m_details;

class Node::ReferencePolisher : public omm::serialization::ReferencePolisher
{
public:
  explicit ReferencePolisher(std::list<ConnectionIds>&& connection_ids, Node& node)
    : m_connection_ids(connection_ids)
    , m_node(node)
  {
  }

private:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override
  {
    QSignalBlocker blocker(m_node.model());
    for (const ConnectionIds& cids : m_connection_ids) {
      Node& node = dynamic_cast<Node&>(*map.at(cids.node_id));
      assert(&node.model() == &m_node.model());
      auto* input = m_node.find_port<InputPort>(cids.input_port);
      auto* output = node.find_port<OutputPort>(cids.output_port);
      if (input != nullptr && output != nullptr) {
        input->connect(output);
      }
    }
  }

  std::list<ConnectionIds> m_connection_ids;
  Node &m_node;
};

Node::Node(NodeModel& model) : PropertyOwner(model.scene()), m_model(model)
{
}

Flag Node::flags() const
{
  return Flag::None;
}

Node::~Node() = default;

std::set<AbstractPort*> Node::ports() const
{
  return util::transform(m_ports, [](const std::unique_ptr<AbstractPort>& p) { return p.get(); });
}

void Node::serialize(serialization::SerializerWorker& worker) const
{
  AbstractPropertyOwner::serialize(worker);
  worker.sub(POS_PTR)->set_value(Vec2f(pos()));

  std::vector<const InputPort*> connection_inputs;
  connection_inputs.reserve(m_ports.size());

  static constexpr auto less_than = [](const AbstractPort* p1, const AbstractPort* p2) {
    static constexpr auto to_tuple = [](const AbstractPort& p) { return std::tuple{p.port_type, p.index}; };
    if (p1 == nullptr || p2 == nullptr) {
      return p1 < p2;
    }
    return to_tuple(*p1) < to_tuple(*p2);  // NOLINT(modernize-use-nullptr)
  };
  const auto ports = util::transform(m_ports, &std::unique_ptr<AbstractPort>::get);
  const auto sorted_ports = serialization::sort<decltype(less_than)>(ports);

  for (const auto& port : sorted_ports) {
    if (const auto* ip = dynamic_cast<const InputPort*>(port); ip != nullptr && ip->connected_output() != nullptr) {
      assert(ip->port_type == PortType::Input);
      connection_inputs.push_back(ip);
    }
  }
  connection_inputs.shrink_to_fit();
  worker.sub(CONNECTIONS_PTR)->set_value(connection_inputs, [](const auto& input, auto& worker_i) {
    const OutputPort& output = *input->connected_output();
    worker_i.sub(INPUT_PORT_PTR)->set_value(input->index);
    worker_i.sub(OUTPUT_PORT_PTR)->set_value(output.index);
    worker_i.sub(CONNECTED_NODE_PTR)->set_value(&output.node);
  });
}

void Node::deserialize(serialization::DeserializerWorker& worker)
{
  AbstractPropertyOwner::deserialize(worker);
  m_pos = worker.sub(POS_PTR)->get<Vec2f>().to_pointf();

  std::list<ConnectionIds> connection_idss;
  worker.sub(CONNECTIONS_PTR)->get_items([&connection_idss](auto& worker_i) {
    ConnectionIds connection_ids{};
    connection_ids.input_port = worker_i.sub(INPUT_PORT_PTR)->get_size_t();
    connection_ids.output_port = worker_i.sub(OUTPUT_PORT_PTR)->get_size_t();
    connection_ids.node_id = worker_i.sub(CONNECTED_NODE_PTR)->get_size_t();
    connection_idss.push_back(connection_ids);
  });
  auto polisher = std::make_unique<ReferencePolisher>(std::move(connection_idss), *this);
  worker.deserializer().register_reference_polisher(std::move(polisher));
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

NodeModel& Node::model() const
{
  return m_model;
}

QString Node::name() const
{
  return QCoreApplication::translate("any-context", type().toStdString().c_str());
}

BackendLanguage Node::language() const
{
  return model().language();
}

QString Node::dangling_input_port_uuid(const InputPort& port) const
{
  return port.uuid();
}

std::set<Node*> Node::successors() const
{
  std::set<Node*> successors;
  for (const auto& port : m_ports) {
    if (port->port_type == PortType::Output) {
      const OutputPort& op = dynamic_cast<OutputPort&>(*port);
      for (const InputPort* ip : op.connected_inputs()) {
        successors.insert(&ip->node);
      }
    }
  }
  return successors;
}

void Node::populate_menu(QMenu&)
{
}

QString Node::title() const
{
  return QCoreApplication::translate("any-context", type().toStdString().c_str());
}

Type Node::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  Q_UNREACHABLE();
  return Type::Invalid;
}

Type Node::input_data_type(const InputPort& port) const
{
  if (OutputPort* op = port.connected_output(); op != nullptr) {
    return op->data_type();
  } else {
    return Type::Invalid;
  }
}

bool Node::accepts_input_data_type(Type type, const InputPort& port, const bool with_cast) const
{
  Q_UNUSED(with_cast)

  // do not perform any type conversions by default.
  // If you want your node to be more flexible, override this method.
  return port.data_type() == type;
}

bool Node::is_valid() const
{
  for (const auto& p : m_ports) {
    if (p->data_type() == Type::Invalid) {
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

QString Node::function_name(std::size_t i) const
{
  return QString("%1_%2").arg(type()).arg(i);
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
      auto own_port = std::move(m_ports.extract(it).value());
      m_model.emit_topology_changed();
      Q_EMIT ports_changed();
      return own_port;
    }
  }
  return nullptr;
}

Property& Node::add_property(const QString& key, std::unique_ptr<Property> property)
{
  return add_property(key, std::move(property), PortType::Both);
}

Property& Node::add_property(const QString& key, std::unique_ptr<Property> property, PortType type)
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
  if (auto* ip = find_port<InputPort>(*property); ip) {
    remove_port(*ip);
  }
  if (auto* op = find_port<OutputPort>(*property); op) {
    remove_port(*op);
  }
  return property;
}

bool Node::copyable() const
{
  return true;
}

const Node::Detail& Node::detail(const QString& name)
{
  return *m_details.at(name);
}

Type Node::fst_con_ptype(const std::vector<InputPort*>& ports, const Type default_t)
{
  const auto get_connected_output = [](const InputPort* ip) { return ip->connected_output(); };
  return ::find_if(
      util::transform(ports, get_connected_output),
      [](const OutputPort* op) { return op != nullptr; },
      [](const OutputPort* op) { return op->data_type(); },
      default_t);
}

}  // namespace omm::nodes
