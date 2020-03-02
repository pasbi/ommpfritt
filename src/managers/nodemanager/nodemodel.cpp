#include "managers/nodemanager/nodemodel.h"
#include "serializers/jsonserializer.h"
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "common.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodecompilerglsl.h"
#include "managers/nodemanager/nodecompilerpython.h"

namespace
{

std::unique_ptr<omm::AbstractNodeCompiler>
make_compiler(omm::AbstractNodeCompiler::Language language, omm::NodeModel& model)
{
  using Language = omm::AbstractNodeCompiler::Language;
  switch (language) {
  case Language::GLSL:
    return std::make_unique<omm::NodeCompilerGLSL>(model);
  case Language::Python:
    return std::make_unique<omm::NodeCompilerPython>(model);
  default:
    Q_UNREACHABLE();
    return nullptr;
  }
}

}  // namespace

namespace omm
{

NodeModel::NodeModel(AbstractNodeCompiler::Language language, Scene& scene)
  : m_scene(scene), m_compiler(make_compiler(language, *this))
{
  init();
  connect(this, SIGNAL(node_added(Node&)), this, SIGNAL(topology_changed()));
  connect(this, SIGNAL(node_removed(Node&)), this, SIGNAL(topology_changed()));
  connect(this, SIGNAL(topology_changed()), m_compiler.get(), SLOT(invalidate()));
}

NodeModel::NodeModel(const NodeModel& other)
  : NodeModel(other.compiler().language, other.m_scene)
{
  init();
  std::ostringstream oss;
  {
    JSONSerializer serializer(oss);
    other.serialize(serializer, "root");
  }
  const std::string str = oss.str();

  {
    QSignalBlocker blocker(this);
    std::istringstream iss(str);
    JSONDeserializer deserializer(iss);
    deserialize(deserializer, "root");
  }
  for (Node* node : nodes()) {
    node->new_id();
  }

  Q_EMIT topology_changed();
}

NodeModel::~NodeModel()
{
}

void NodeModel::init()
{
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
  const auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                               [&node](const std::unique_ptr<Node>& candidate)
  {
    return candidate.get() == &node;
  });

  if (it != m_nodes.end()) {
    assert(node.is_free());
    auto node = std::move(m_nodes.extract(it).value());
    Q_EMIT topology_changed();
    Q_EMIT node_removed(*node);
    return node;
  } else {
    return nullptr;
  }
}

std::set<Node*> NodeModel::nodes() const
{
  return ::transform<Node*>(m_nodes, [](const std::unique_ptr<Node>& node) {
    return node.get();
  });
}

void NodeModel::serialize(AbstractSerializer& serializer, const Serializable::Pointer& ptr) const
{
  serializer.start_array(m_nodes.size(), Serializable::make_pointer(ptr, NODES_POINTER));
  std::size_t i = 0;
  for (auto&& node : m_nodes) {
    const auto node_ptr = Serializable::make_pointer(ptr, NODES_POINTER, i);
    node->serialize(serializer, node_ptr);
    serializer.set_value(node->type(), make_pointer(node_ptr, TYPE_POINTER));
    i += 1;
  }
  serializer.end_array();
}

void NodeModel::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& ptr)
{
  QSignalBlocker blocker(this);
  const auto n = deserializer.array_size(Serializable::make_pointer(ptr, NODES_POINTER));
  for (size_t i = 0; i < n; ++i) {
    const auto node_ptr = Serializable::make_pointer(ptr, NODES_POINTER, i);
    const auto type = deserializer.get_string(make_pointer(node_ptr, TYPE_POINTER));
    auto node = Node::make(type, *this);
    node->deserialize(deserializer, node_ptr);
    add_node(std::move(node));
  }
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

AbstractNodeCompiler& NodeModel::compiler() const
{
  return *m_compiler;
}

void NodeModel::set_error(const QString& error)
{
  m_error = error;
}

bool NodeModel::can_connect(const AbstractPort& a, const AbstractPort& b) const
{
  const InputPort* in;
  const OutputPort* out;

  if (a.port_type == PortType::Input && b.port_type == PortType::Output) {
    in = static_cast<const InputPort*>(&a);
    out = static_cast<const OutputPort*>(&b);
  } else if (a.port_type == PortType::Output && b.port_type == PortType::Input) {
    in = static_cast<const InputPort*>(&b);
    out = static_cast<const OutputPort*>(&a);
  } else {
    return false;
  }

  // It's a input and output. Check cycles, types, etc.
  return can_connect(*out, *in);
}

bool NodeModel::can_connect(const OutputPort& a, const InputPort& b) const
{
  return !find_path(b.node, a.node) && b.accepts_data_type(a.data_type());
}

}  // namespace omm
