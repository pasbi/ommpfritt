#include "managers/nodemanager/nodescene.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodeitem.h"

namespace omm
{

NodeScene::NodeScene()
{
  clear();
}

NodeScene::~NodeScene()
{

}

void NodeScene::set_model(NodeModel* model)
{
  clear();
  m_model = model;
  for (Node* node : model->nodes()) {
    add_node(*node);
  }

  m_node_connections.push_back(
        connect(m_model, SIGNAL(node_added(Node&)), this, SLOT(add_node(Node&))));
  m_node_connections.push_back(
        connect(m_model, SIGNAL(node_about_to_be_removed(Node&)), this, SLOT(remove_node(Node&))));
}

void NodeScene::clear()
{
  for (auto&& [node, node_item] : m_node_items) {
    removeItem(node_item.get());
  }
  for (auto& connection : m_node_connections) {
    QObject::disconnect(connection);
  }

  m_node_connections.clear();
  m_node_items.clear();
}

NodeItem& NodeScene::node_item(Node& node) const
{
  return *m_node_items.at(&node);
}

void NodeScene::add_node(Node& node)
{
  auto node_item = std::make_unique<NodeItem>(node);
  addItem(node_item.get());
  m_node_items.insert({ &node, std::move(node_item) });
}

void NodeScene::remove_node(Node& node)
{
  auto it = m_node_items.find(&node);
  removeItem(it->second.get());
  m_node_items.erase(it);
}


}  // namespace omm
