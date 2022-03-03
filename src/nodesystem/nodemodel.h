#pragma once

#include "cachedgetter.h"
#include "common.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/port.h"
#include <QObject>
#include <memory>
#include <set>

namespace omm
{

class Scene;

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

}  // namespace omm

namespace omm::nodes
{

class AbstractNodeCompiler;
class Node;
class AbstractPort;
class OutputPort;
class InputPort;
class FragmentNode;

class NodeModel : public QObject
{
  Q_OBJECT
public:
  explicit NodeModel(BackendLanguage language, Scene* scene);
  static std::unique_ptr<NodeModel> make(BackendLanguage language, Scene* scene);
  NodeModel(const NodeModel& other);
  ~NodeModel() override;
  NodeModel(NodeModel&&) = delete;
  NodeModel& operator=(NodeModel&&) = delete;
  NodeModel& operator=(const NodeModel&) = delete;

  Node& add_node(std::unique_ptr<Node> node);
  std::unique_ptr<Node> extract_node(Node& node);
  [[nodiscard]] std::set<Node*> nodes() const;
  [[nodiscard]] bool can_connect(const AbstractPort& a, const AbstractPort& b) const;
  [[nodiscard]] bool can_connect(const OutputPort& a, const InputPort& b) const;
  using QObject::connect;

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  static constexpr auto NODES_POINTER = "nodes";
  static constexpr auto TYPE_POINTER = "type";

  bool find_path(const Node& start, const Node& end, std::list<const Node*>& path) const;
  bool find_path(std::list<const Node*>& path, const Node& end) const;
  [[nodiscard]] bool find_path(const Node& start, const Node& end) const;
  [[nodiscard]] bool types_compatible(const QString& from, const QString& to) const;

  [[nodiscard]] std::set<AbstractPort*> ports() const;
  template<typename PortT> std::set<PortT*> ports() const
  {
    auto ports = this->ports();
    std::erase_if(ports, [](AbstractPort* p) { return p->port_type != PortT::PORT_TYPE; });
    return util::transform(ports, [](AbstractPort* p) { return static_cast<PortT*>(p); });
  }

  [[nodiscard]] BackendLanguage language() const
  {
    return m_compiler->language;
  }

  [[nodiscard]] Scene* scene() const;

  [[nodiscard]] AbstractNodeCompiler& compiler() const;
  [[nodiscard]] QString error() const
  {
    return m_error;
  }

  friend class TopologyChangeSignalBlocker;
  class TopologyChangeSignalBlocker
  {
  public:
    explicit TopologyChangeSignalBlocker(NodeModel& model);
    ~TopologyChangeSignalBlocker();
    TopologyChangeSignalBlocker() = delete;
    TopologyChangeSignalBlocker(const TopologyChangeSignalBlocker&) = delete;
    TopologyChangeSignalBlocker(TopologyChangeSignalBlocker&&) = delete;
    TopologyChangeSignalBlocker& operator=(const TopologyChangeSignalBlocker&) = delete;
    TopologyChangeSignalBlocker& operator=(TopologyChangeSignalBlocker&&) = delete;

  private:
    NodeModel& m_model;
  };

public:
  [[nodiscard]] bool is_enabled() const;
  void set_error(const QString& error);

Q_SIGNALS:
  void topology_changed();
  void node_added(omm::nodes::Node&);
  void node_removed(omm::nodes::Node&);

public:
  void emit_topology_changed();

private:
  std::set<std::unique_ptr<Node>> m_nodes;
  Scene* m_scene = nullptr;
  void init();
  std::unique_ptr<AbstractNodeCompiler> m_compiler;
  QString m_error = "";
  FragmentNode* m_fragment_node = nullptr;
  bool m_emit_topology_changed_blocked = false;
  const bool m_is_enabled;
};

}  // namespace omm::nodes
