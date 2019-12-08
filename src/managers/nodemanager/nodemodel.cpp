#include "managers/nodemanager/nodemodel.h"
#include "common.h"
#include "managers/nodemanager/nodes/gradientnode.h"
#include "managers/nodemanager/port.h"

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

}  // namespace omm
