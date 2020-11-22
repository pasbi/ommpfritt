#pragma once

#include <QGraphicsScene>
#include <map>
#include <memory>
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
  Q_OBJECT
public:
  explicit NodeScene(Scene& scene);
  ~NodeScene();
  NodeItem& node_item(Node& node) const;
  void set_model(omm::NodeModel* model);
  NodeModel* model() const
  {
    return m_model;
  }
  std::set<Node*> selected_nodes() const;
  Scene& scene;

  void clear();

public Q_SLOTS:
  void add_node(omm::Node& node, bool select = true);
  void remove_node(omm::Node& node);

private:
  std::map<Node*, std::unique_ptr<NodeItem>> m_node_items;
  NodeModel* m_model = nullptr;
  bool m_block_selection_change_notification = false;
  std::list<QMetaObject::Connection> m_scene_model_connections;
};

}  // namespace omm
