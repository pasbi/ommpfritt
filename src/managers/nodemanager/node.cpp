#include "managers/nodemanager/node.h"
#include "common.h"

namespace omm
{

Node::Node(Scene* scene) : PropertyOwner(scene)
{
}

Node::~Node()
{
}

std::set<Port*> Node::ports() const
{
  return ::transform<Port*>(m_ports, [](const std::unique_ptr<Port>& p) {
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

  std::vector<const InputPort*> connection_targets;
  connection_targets.reserve(m_ports.size());
  for (const auto& port : m_ports) {
    if (port->is_input && static_cast<InputPort&>(*port).connection_origin() != nullptr) {
      connection_targets.push_back(static_cast<const InputPort*>(port.get()));
    }
  }
  connection_targets.shrink_to_fit();
  const auto connections_ptr = make_pointer(root, CONNECTIONS_PTR);
  serializer.start_array(connection_targets.size(), connections_ptr);
  for (std::size_t i = 0; i < connection_targets.size(); ++i) {
    const auto iptr = make_pointer(connections_ptr, i);
    const InputPort& target = *connection_targets.at(i);
    const OutputPort& origin = *target.connection_origin();
    serializer.set_value(target.index, make_pointer(iptr, TARGET_PORT_PTR));
    serializer.set_value(origin.index, make_pointer(iptr, ORIGIN_PORT_PTR));
    serializer.set_value(&origin.node, make_pointer(iptr, CONNECTED_NODE_PTR));
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
    connection_ids.target_port = deserializer.get_size_t(make_pointer(iptr, TARGET_PORT_PTR));
    connection_ids.origin_port = deserializer.get_size_t(make_pointer(iptr, ORIGIN_PORT_PTR));
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

std::set<Node*> Node::successors() const
{
  std::set<Node*> successors;
  for (const auto& port : m_ports) {
    if (!port->is_input) {
      const OutputPort& op = static_cast<OutputPort&>(*port);
      for (const InputPort* ip : op.connected_ports()) {
        successors.insert(&ip->node);
      }
    }
  }
  return successors;
}

void Node::update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map)
{
  for (const ConnectionIds& cids : m_connection_ids) {
    Node& node = static_cast<Node&>(*map.at(cids.node_id));
    InputPort& target = *find_port<InputPort>(cids.target_port);
    OutputPort& origin = *node.find_port<OutputPort>(cids.origin_port);
    target.connect(&origin);
  }
}

}  // namespace omm
