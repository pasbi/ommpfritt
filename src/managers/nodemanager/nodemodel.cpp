#include "managers/nodemanager/nodemodel.h"
#include "common.h"
#include "managers/nodemanager/nodes/gradientnode.h"
#include "managers/nodemanager/port.h"

namespace
{
template<typename Port, typename InputPort, typename OutputPort>
bool sort_ports(Port a, Port b, InputPort &in, OutputPort &out)
{
  if (a->is_input && !b->is_input) {
    out = static_cast<OutputPort>(b);
    in = static_cast<InputPort>(a);
    return true;
  } else if (!a->is_input && b->is_input) {
    out = static_cast<OutputPort>(a);
    in = static_cast<InputPort>(b);
    return true;
  } else {
    return false;
  }
}

}  // namespace

namespace omm
{

NodeModel::NodeModel(Scene* scene) : m_scene(scene)
{
  add_node(std::make_unique<GradientNode>(scene));
  add_node(std::make_unique<GradientNode>(scene));
}

NodeModel::~NodeModel()
{
}

Node& NodeModel::add_node(std::unique_ptr<Node> node)
{
  Node& ref = *node;
  connect(&ref, SIGNAL(pos_changed()), this, SIGNAL(appearance_changed()));
  node->set_model(this);
  m_nodes.insert(std::move(node));
  return ref;
}

std::unique_ptr<Node> NodeModel::extract_node(Node& node)
{
  const auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                               [&node](const std::unique_ptr<Node>& candidate)
  {
    return candidate.get() == &node;
  });

  if (it != m_nodes.end()) {
    auto node = std::move(m_nodes.extract(it).value());
    disconnect(node.get(), SIGNAL(pos_changed()), this, SIGNAL(appearance_changed()));
    node->set_model(nullptr);
    return node;
  } else {
    return nullptr;
  }
}

std::set<Node*> NodeModel::nodes() const
{
  return ::transform<Node*>(m_nodes, [](const std::unique_ptr<Node>& node) {
    return node.get();
  });
}

void NodeModel::serialize(AbstractSerializer& serializer, const Serializable::Pointer& ptr) const
{
  serializer.start_array(m_nodes.size(), Serializable::make_pointer(ptr, NODES_POINTER));
  std::size_t i = 0;
  for (auto&& node : m_nodes) {
    const auto node_ptr = Serializable::make_pointer(ptr, NODES_POINTER, i);
    node->serialize(serializer, node_ptr);
    serializer.set_value(node->type(), make_pointer(node_ptr, TYPE_POINTER));
    i += 1;
  }
  serializer.end_array();
}

void NodeModel::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& ptr)
{
  const auto n = deserializer.array_size(Serializable::make_pointer(ptr, NODES_POINTER));
  for (size_t i = 0; i < n; ++i) {
    const auto node_ptr = Serializable::make_pointer(ptr, NODES_POINTER, i);
    const auto type = deserializer.get_string(make_pointer(node_ptr, TYPE_POINTER));
    auto node = Node::make(type, m_scene);
    node->deserialize(deserializer, node_ptr);
    add_node(std::move(node));
  }
}

bool NodeModel::find_path(const Node& start, const Node& end, std::list<const Node*>& path) const
{
  path.clear();
  path.push_back(&start);
  return find_path(path, end);
}

bool NodeModel::find_path(std::list<const Node*>& path, const Node& end) const
{
  assert(!path.empty());
  const Node& last = *path.back();
  if (&last == &end) {
    return true;
  } else {
    for (const Node* node : last.successors()) {
      if (::contains(path, node)) {
        LERROR << "Unexpected cycle.";
      } else {
        path.push_back(node);
        if (find_path(path, end)) {
          return true;
        }
        path.pop_back();
      }
    }
  }
  return false;
}

bool NodeModel::find_path(const Node& start, const Node& end) const
{
  std::list<const Node*> path;
  return find_path(start, end, path);
}

std::set<Port*> NodeModel::ports() const
{
  std::set<Port*> ports;
  for (const auto& node : m_nodes) {
    const auto ps = node->ports();
    ports.insert(ps.begin(), ps.end());
  }
  return ports;
}

bool NodeModel::can_connect(const Port& a, const Port& b) const
{
  const InputPort* in;
  const OutputPort* out;
  if (sort_ports(&a, &b, in, out)) {
    return can_connect(*out, *in);
  } else {
    return false;
  }
}

bool NodeModel::can_connect(const OutputPort& a, const InputPort& b) const
{
  return !find_path(b.node, a.node);
}

void NodeModel::connect(Port& a, Port& b)
{
  InputPort* in;
  OutputPort* out;
  if (sort_ports(&a, &b, in, out)) {
    assert(can_connect(*out, *in));
    in->connect(out);
  }
}


}  // namespace omm
