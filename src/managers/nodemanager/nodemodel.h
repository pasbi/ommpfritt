#pragma once

#include <QObject>
#include <set>
#include <memory>

namespace omm
{

class Node;
class OutputPort;
class InputPort;

class NodeModel : public QObject
{
  Q_OBJECT
public:
  explicit NodeModel();
  ~NodeModel();

  Node& add_node(std::unique_ptr<Node> node);
  std::unique_ptr<Node> extract_node(Node& node);
  std::set<Node*> nodes() const;

Q_SIGNALS:
  void appearance_changed();

private:
  std::set<std::unique_ptr<Node>> m_nodes;
};


}  // namespace omm
