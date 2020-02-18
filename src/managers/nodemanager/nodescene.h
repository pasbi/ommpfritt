#pragma once

#include <memory>
#include <QGraphicsScene>
#include <set>

namespace omm
{

class NodeModel;
class Node;
class NodeItem;
class PortItem;
class AbstractPort;

class NodeScene : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit NodeScene();
  ~NodeScene();
  void set_model(NodeModel* model);
  void clear();
  NodeItem& node_item(Node& node) const;
  NodeModel* model() const { return m_model; }

private:
  NodeModel* m_model;
  std::map<Node*, std::unique_ptr<NodeItem>> m_node_items;
  std::list<QMetaObject::Connection> m_node_connections;
  std::map<const AbstractPort*, PortItem*> m_port_map;

private Q_SLOTS:
  void add_node(Node& node);
  void remove_node(Node& node);
};

}  // namespace omm
