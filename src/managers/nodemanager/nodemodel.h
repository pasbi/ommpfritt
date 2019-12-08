#pragma once

#include "aspects/serializable.h"
#include <QObject>
#include <set>
#include <memory>

namespace omm
{

class Node;
class Port;
class OutputPort;
class InputPort;
class Scene;

class NodeModel : public QObject, public Serializable
{
  Q_OBJECT
public:
  explicit NodeModel(Scene* scene);
  ~NodeModel();

  Node& add_node(std::unique_ptr<Node> node);
  std::unique_ptr<Node> extract_node(Node& node);
  std::set<Node*> nodes() const;
  bool can_connect(const Port& a, const Port& b) const;
  bool can_connect(const OutputPort& a, const InputPort& b) const;
  void connect(Port& a, Port& b);
  using QObject::connect;

  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer&deserializer, const Pointer&ptr) override;

  static constexpr auto NODES_POINTER = "nodes";
  static constexpr auto TYPE_POINTER = "type";

  bool find_path(const Node& start, const Node& end, std::list<const Node*>& path) const;
  bool find_path(std::list<const Node*>& path, const Node& end) const;
  bool find_path(const Node& start, const Node& end) const;

Q_SIGNALS:
  void appearance_changed();

private:
  std::set<std::unique_ptr<Node>> m_nodes;
  Scene* m_scene;
};


}  // namespace omm
