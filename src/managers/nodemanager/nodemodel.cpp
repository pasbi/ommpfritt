#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/nodeitem.h"
#include "serializers/jsonserializer.h"
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "common.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"

namespace omm
{

NodeModel::NodeModel(AbstractNodeCompiler::Language language, Scene& scene)
  : m_scene(scene), m_language(language)
{
  init();
  connect(this, &NodeModel::selectionChanged, [this]() {
    m_scene.set_selection(::transform<AbstractPropertyOwner*>(selected_nodes()));
  });
}

NodeModel::NodeModel(const NodeModel& other)
  : NodeModel(other.m_language, other.m_scene)
{
  init();
  std::ostringstream oss;
  {
    JSONSerializer serializer(oss);
    other.serialize(serializer, "root");
  }
  const std::string str = oss.str();
  LINFO << str;

  {
    QSignalBlocker blocker(this);
    std::istringstream iss(str);
    JSONDeserializer deserializer(iss);
    deserialize(deserializer, "root");
  }

  Q_EMIT topology_changed();
}

NodeModel::~NodeModel()
{
  clearSelection();
}

std::set<Node*> NodeModel::selected_nodes() const
{
  return ::filter_if(nodes(), [this](Node* node) {
    return node_item(*node).isSelected();
  });
}

void NodeModel::set_status(NodeModel::Status status)
{
  if (m_status != status) {
    m_status = status;
    Q_EMIT appearance_changed();
  }
}

void NodeModel::init()
{
  connect(this, SIGNAL(node_shape_changed()), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(topology_changed()), this, SIGNAL(node_shape_changed()));
  connect(&scene().message_box(), &MessageBox::property_value_changed,
          [this](AbstractPropertyOwner& apo, const QString& key, Property&)
  {
    Q_UNUSED(key)
    if (apo.kind == Kind::Node && &static_cast<const Node&>(apo).model() == this)
    {
      Q_EMIT appearance_changed();
    }
  });
}

Node& NodeModel::add_node(std::unique_ptr<Node> node)
{
  Node& node_ref = *node;
  connect(&node_ref, SIGNAL(pos_changed(const QPointF&)), this, SIGNAL(appearance_changed()));
  assert(&node->model() == this);
  m_nodes.insert(std::move(node));
  Q_EMIT topology_changed();

  auto node_item = std::make_unique<NodeItem>(node_ref);
  auto& node_item_ref = *node_item;
  addItem(node_item.get());
  m_node_items.insert({ &node_ref, std::move(node_item) });

  clearSelection();
  node_item_ref.setSelected(true);

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
    {
      auto it = m_node_items.find(&node);
      removeItem(it->second.get());
      m_node_items.erase(it);
    }

    assert(node.is_free());
    auto node = std::move(m_nodes.extract(it).value());
    disconnect(node.get(), SIGNAL(pos_changed(const QPointF&)), this, SIGNAL(appearance_changed()));
    Q_EMIT topology_changed();
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

void NodeModel::notify_appearance_changed()
{
  Q_EMIT appearance_changed();
}

void NodeModel::notify_topology_changed()
{
  Q_EMIT topology_changed();
}

void NodeModel::notify_node_shape_changed()
{
  Q_EMIT node_shape_changed();
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

void NodeModel::clear()
{
  for (auto&& [node, node_item] : m_node_items) {
    removeItem(node_item.get());
  }
  for (auto& connection : m_node_connections) {
    QObject::disconnect(connection);
  }

  m_node_connections.clear();
  m_node_items.clear();
}

NodeItem& NodeModel::node_item(Node& node) const
{
  return *m_node_items.at(&node);
}

}  // namespace omm
