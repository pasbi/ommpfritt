#pragma once

#include "aspects/serializable.h"
#include "cachedgetter.h"
#include "common.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/port.h"
#include <QObject>
#include <memory>
#include <set>

namespace omm
{
class Node;
class AbstractPort;
class OutputPort;
class InputPort;
class Scene;
class FragmentNode;

class NodeModel
    : public QObject
    , public Serializable
{
  Q_OBJECT
public:
  explicit NodeModel(AbstractNodeCompiler::Language language, Scene& scene);
  static std::unique_ptr<NodeModel> make(AbstractNodeCompiler::Language language, Scene& scene);
  NodeModel(const NodeModel& other);
  ~NodeModel();

  Node& add_node(std::unique_ptr<Node> node);
  std::unique_ptr<Node> extract_node(Node& node);
  std::set<Node*> nodes() const;
  bool can_connect(const AbstractPort& a, const AbstractPort& b) const;
  bool can_connect(const OutputPort& a, const InputPort& b) const;
  using QObject::connect;

  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& ptr) override;

  static constexpr auto NODES_POINTER = "nodes";
  static constexpr auto TYPE_POINTER = "type";

  bool find_path(const Node& start, const Node& end, std::list<const Node*>& path) const;
  bool find_path(std::list<const Node*>& path, const Node& end) const;
  bool find_path(const Node& start, const Node& end) const;
  bool types_compatible(const QString& from, const QString& to) const;

  std::set<AbstractPort*> ports() const;
  template<typename PortT> std::set<PortT*> ports() const
  {
    static const auto pred = [](AbstractPort* p) { return p->port_type == PortT::PORT_TYPE; };
    static const auto conv = [](AbstractPort* p) { return static_cast<PortT*>(p); };
    return ::transform<PortT*>(::filter_if(ports(), pred), conv);
  }

  AbstractNodeCompiler::Language language() const
  {
    return m_compiler->language;
  }
  Scene& scene() const
  {
    return m_scene;
  }
  AbstractNodeCompiler& compiler() const;
  QString error() const
  {
    return m_error;
  }

  friend class TopologyChangeSignalBlocker;
  class TopologyChangeSignalBlocker
  {
  public:
    explicit TopologyChangeSignalBlocker(NodeModel& model);
    ~TopologyChangeSignalBlocker();

  private:
    NodeModel& m_model;
  };

public Q_SLOTS:
  void set_error(const QString& error);

Q_SIGNALS:
  void topology_changed();
  void node_added(Node&);
  void node_removed(Node&);

public Q_SLOTS:
  void emit_topology_changed();

private:
  std::set<std::unique_ptr<Node>> m_nodes;
  Scene& m_scene;
  void init();
  std::unique_ptr<AbstractNodeCompiler> m_compiler;
  QString m_error = "";
  FragmentNode* m_fragment_node = nullptr;
  bool m_emit_topology_changed_blocked = false;
};

}  // namespace omm
