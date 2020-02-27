#pragma once

#include <QGraphicsScene>
#include <memory>
#include <map>
#include <set>

namespace omm
{

class Node;
class NodeItem;
class PortItem;
class AbstractPort;
class NodeModel;
class Scene;

class NodeScene : public QGraphicsScene
{
public:
  explicit NodeScene(Scene& scene);
  ~NodeScene();
  NodeItem& node_item(Node& node) const;
  void set_model(NodeModel* model);
  NodeModel* model() const { return m_model; }
  std::set<Node*> selected_nodes() const;
  Scene& scene;

  void add_node(Node& node, bool select = true);
  void remove_node(Node& node);
  void clear();

private:
  std::map<Node*, std::unique_ptr<NodeItem>> m_node_items;
  NodeModel* m_model = nullptr;
  bool m_block_selection_change_notification = false;
};

}  // namespace omm
