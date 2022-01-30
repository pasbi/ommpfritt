#pragma once

#include <QGraphicsScene>
#include <map>
#include <memory>
#include <set>

namespace omm
{

class NodeItem;
class PortItem;

namespace nodes
{
class AbstractPort;
class NodeModel;
class Node;
}  // namespace nodes

class Scene;

class NodeScene : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit NodeScene(Scene& scene);
  ~NodeScene() override;
  NodeScene(NodeScene&&) = delete;
  NodeScene(const NodeScene&) = delete;
  NodeScene& operator=(NodeScene&&) = delete;
  NodeScene& operator=(const NodeScene&) = delete;
  NodeItem& node_item(nodes::Node& node) const;
  void set_model(omm::nodes::NodeModel* model);
  [[nodiscard]] nodes::NodeModel* model() const
  {
    return m_model;
  }
  [[nodiscard]] std::set<nodes::Node*> selected_nodes() const;
  Scene& scene;

  void clear();

public:
  void add_node(omm::nodes::Node& node, bool select = true);
  void remove_node(omm::nodes::Node& node);

private:
  std::map<nodes::Node*, std::unique_ptr<NodeItem>> m_node_items;
  nodes::NodeModel* m_model = nullptr;
  bool m_block_selection_change_notification = false;
  std::list<QMetaObject::Connection> m_scene_model_connections;
};

}  // namespace omm
