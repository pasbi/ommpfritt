#include "managers/nodemanager/nodescene.h"
#include <QTimer>
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "scene/scene.h"
#include "managers/nodemanager/nodeitem.h"
#include "scene/messagebox.h"

namespace omm
{

NodeScene::NodeScene(Scene& scene) : scene(scene)
{
  connect(this, &NodeScene::selectionChanged, [&scene, this]() {
    if (!m_block_selection_change_notification) {
      scene.set_selection(::transform<AbstractPropertyOwner*>(selected_nodes()));
    }
  });
  connect(&scene.message_box(), &MessageBox::property_value_changed,
          this, [this](AbstractPropertyOwner& owner, const QString&, Property&)
  {
    if (Node* node = kind_cast<Node*>(&owner); node != nullptr && &node->model() == m_model) {
      QTimer::singleShot(0, [this]() { update(); });
    }
  });
}

NodeScene::~NodeScene()
{
  clearSelection();
}

void NodeScene::set_model(NodeModel* model)
{
  m_block_selection_change_notification = true;
  clear();
  if (m_model) {
    disconnect(&m_model->compiler(), SIGNAL(compilation_failed(QString)), this, SLOT(update()));
    disconnect(&m_model->compiler(), SIGNAL(compilation_succeeded(QString)), this, SLOT(update()));
    disconnect(m_model, SIGNAL(node_removed(Node&)), this, SLOT(remove_node(Node&)));
    disconnect(m_model, SIGNAL(node_added(Node&)), this, SLOT(add_node(Node&)));
  }
  m_model = model;
  if (m_model) {
    connect(m_model, SIGNAL(node_added(Node&)), this, SLOT(add_node(Node&)));
    connect(m_model, SIGNAL(node_removed(Node&)), this, SLOT(remove_node(Node&)));
    connect(&m_model->compiler(), SIGNAL(compilation_succeeded(QString)), this, SLOT(update()));
    connect(&m_model->compiler(), SIGNAL(compilation_failed(QString)), this, SLOT(update()));
  }
  if (m_model != nullptr) {
    for (Node* node : model->nodes()) {
      add_node(*node, false);
    }
  }
  m_block_selection_change_notification = false;
}

std::set<Node*> NodeScene::selected_nodes() const
{
  return ::filter_if(m_model->nodes(), [this](Node* node) {
    return node_item(*node).isSelected();
  });
}

void NodeScene::add_node(Node& node, bool select)
{
  auto node_item = std::make_unique<NodeItem>(node);
  auto& node_item_ref = *node_item;
  addItem(node_item.get());
  m_node_items.insert({ &node, std::move(node_item) });

  if (select) {
    clearSelection();
    node_item_ref.setSelected(true);
  }
}

void NodeScene::remove_node(Node& node)
{
  auto it = m_node_items.find(&node);
  removeItem(it->second.get());
  m_node_items.erase(it);
}

void NodeScene::clear()
{
  m_node_items.clear();
  QGraphicsScene::clear();
}

NodeItem& NodeScene::node_item(Node& node) const
{
  return *m_node_items.at(&node);
}

}  // namespace
