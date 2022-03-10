#include "nodesystem/nodemodel.h"
#include "common.h"
#include "main/application.h"
#include "main/options.h"
#include "nodesystem/node.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/nodecompilerpython.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/port.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "serializers/json/jsonserializer.h"
#include "serializers/json/jsondeserializer.h"


namespace
{

bool is_enabled(const omm::nodes::BackendLanguage language)
{
  bool have_opengl = !omm::Application::instance().options().have_opengl;
  return language != omm::nodes::BackendLanguage::GLSL || !have_opengl;
}

std::unique_ptr<omm::nodes::AbstractNodeCompiler>
make_compiler(omm::nodes::BackendLanguage language, omm::nodes::NodeModel& model)
{
  using Language = omm::nodes::BackendLanguage;
  switch (language) {
  case Language::GLSL:
    return std::make_unique<omm::nodes::NodeCompilerGLSL>(model);
  case Language::Python:
    return std::make_unique<omm::nodes::NodeCompilerPython>(model);
  default:
    Q_UNREACHABLE();
    return nullptr;
  }
}

}  // namespace

namespace omm::nodes
{

NodeModel::NodeModel(BackendLanguage language, Scene* scene)
    : m_scene(scene), m_compiler(make_compiler(language, *this))
    , m_is_enabled(::is_enabled(language))
{
  init();
}

NodeModel::NodeModel(const NodeModel& other) : NodeModel(other.compiler().language, other.m_scene)
{
  nlohmann::json store;
  {
    serialization::JSONSerializer serializer(store);
    auto worker = serializer.worker();
    other.serialize(*worker);
  }

  {
    QSignalBlocker blocker(this);
    serialization::JSONDeserializer deserializer(store);
    auto worker = deserializer.worker();
    deserialize(*worker);
  }

  for (Node* node : nodes()) {
    node->new_id();
  }

  emit_topology_changed();
}

NodeModel::~NodeModel() = default;

void NodeModel::init()
{
  connect(this, &NodeModel::node_added, this, &NodeModel::emit_topology_changed);
  connect(this, &NodeModel::node_removed, this, &NodeModel::emit_topology_changed);
  connect(this, &NodeModel::topology_changed, m_compiler.get(), &AbstractNodeCompiler::invalidate);
  if (m_compiler->language == BackendLanguage::GLSL) {
    auto fragment_node = std::make_unique<FragmentNode>(*this);
    m_fragment_node = fragment_node.get();
    add_node(std::move(fragment_node));
  }
}

Node& NodeModel::add_node(std::unique_ptr<Node> node)
{
  Node& node_ref = *node;
  assert(&node->model() == this);
  m_nodes.insert(std::move(node));
  Q_EMIT node_added(node_ref);

  return node_ref;
}

std::unique_ptr<Node> NodeModel::extract_node(Node& node)
{
  const auto it = std::find_if(
      m_nodes.begin(),
      m_nodes.end(),
      [&node](const std::unique_ptr<Node>& candidate) { return candidate.get() == &node; });

  if (it != m_nodes.end()) {
    auto own_node = std::move(m_nodes.extract(it).value());
    Q_EMIT node_removed(*own_node);
    return own_node;
  } else {
    return nullptr;
  }
}

std::set<Node*> NodeModel::nodes() const
{
  return util::transform(m_nodes, [](const std::unique_ptr<Node>& node) { return node.get(); });
}

void NodeModel::serialize(serialization::SerializerWorker& worker) const
{
  static constexpr auto less_than = [](const Node* n1, const Node* n2) { return n1->id() < n2->id(); };
  const auto nodes = util::transform(m_nodes, &std::unique_ptr<Node>::get);
  const auto sorted_nodes = serialization::sort<decltype(less_than)>(nodes);
  worker.sub(NODES_POINTER)->set_value(sorted_nodes, [](const auto& node, auto& worker_i) {
    worker_i.sub(TYPE_POINTER)->set_value(node->type());
    node->serialize(worker_i);
  });
}

void NodeModel::deserialize(serialization::DeserializerWorker& worker)
{
  QSignalBlocker blocker(this);
  worker.sub(NODES_POINTER)->get_items([this](auto& worker_i) {
    const auto type = worker_i.sub(TYPE_POINTER)->get_string();
    if (type == FragmentNode::TYPE) {
      assert(m_fragment_node != nullptr);
      m_fragment_node->deserialize(worker_i);
    } else {
      auto node = Node::make(type, *this);
      node->deserialize(worker_i);
      add_node(std::move(node));
    }
  });

  // Nodes are not yet connected. They will be connected when the Deserializer gets destroyed.
  connect(&worker.deserializer(), &serialization::AbstractDeserializer::destroyed,
          this, &NodeModel::emit_topology_changed);
}

bool NodeModel::find_path(const Node& start, const Node& end, std::list<const Node*>& path) const
{
  path.clear();
  path.push_back(&start);
  return find_path(path, end);
}

bool NodeModel::find_path(std::list<const Node*>& path, const Node& end) const
{
  assert(!path.empty());
  const Node& last = *path.back();
  if (&last == &end) {
    return true;
  } else {
    for (const Node* node : last.successors()) {
      if (::contains(path, node)) {
        LERROR << "Unexpected cycle.";
      } else {
        path.push_back(node);
        if (find_path(path, end)) {
          return true;
        }
        path.pop_back();
      }
    }
  }
  return false;
}

bool NodeModel::find_path(const Node& start, const Node& end) const
{
  std::list<const Node*> path;
  return find_path(start, end, path);
}

std::set<AbstractPort*> NodeModel::ports() const
{
  std::set<AbstractPort*> ports;
  for (const auto& node : m_nodes) {
    const auto ps = node->ports();
    ports.insert(ps.begin(), ps.end());
  }
  return ports;
}

Scene* NodeModel::scene() const
{
  return m_scene;
}

AbstractNodeCompiler& NodeModel::compiler() const
{
  return *m_compiler;
}

bool NodeModel::is_enabled() const
{
  return m_is_enabled;
}

void NodeModel::set_error(const QString& error)
{
  m_error = error;
}

bool NodeModel::can_connect(const AbstractPort& a, const AbstractPort& b) const
{
  const InputPort* in = nullptr;
  const OutputPort* out = nullptr;

  if (a.port_type == PortType::Input && b.port_type == PortType::Output) {
    in = dynamic_cast<const InputPort*>(&a);
    out = dynamic_cast<const OutputPort*>(&b);
  } else if (a.port_type == PortType::Output && b.port_type == PortType::Input) {
    in = dynamic_cast<const InputPort*>(&b);
    out = dynamic_cast<const OutputPort*>(&a);
  } else {
    return false;
  }

  // It's a input and output. Check cycles, types, etc.
  return can_connect(*out, *in);
}

bool NodeModel::can_connect(const OutputPort& a, const InputPort& b) const
{
  return !find_path(b.node, a.node) && b.accepts_data_type(a.data_type(), true);
}

NodeModel::TopologyChangeSignalBlocker::TopologyChangeSignalBlocker(NodeModel& model)
    : m_model(model)
{
  m_model.m_emit_topology_changed_blocked = true;
}

NodeModel::TopologyChangeSignalBlocker::~TopologyChangeSignalBlocker()
{
  m_model.m_emit_topology_changed_blocked = false;
}

void NodeModel::emit_topology_changed()
{
  if (!m_emit_topology_changed_blocked) {
    Q_EMIT topology_changed();
  }
}

}  // namespace omm::nodes
