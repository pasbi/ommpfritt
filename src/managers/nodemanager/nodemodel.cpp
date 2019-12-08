#include "managers/nodemanager/nodemodel.h"
#include "common.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/port.h"

namespace
{

std::unique_ptr<omm::Node> make_node()
{
  auto node = std::make_unique<omm::Node>();
  node->add_port<omm::InputPort>("iA");
  node->add_port<omm::InputPort>("djohfiudeowh");
  node->add_port<omm::InputPort>("iCdewfpoerujio");
  node->add_port<omm::OutputPort>("oA");
  node->add_port<omm::OutputPort>("oBdweferetgvjkaljewqfiorejhafirhfeoh");
  node->add_port<omm::OutputPort>("oC");
  return node;
}

}  // namespace

namespace omm
{

NodeModel::NodeModel()
{
  add_node(make_node());
  add_node(make_node());
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

}  // namespace omm
