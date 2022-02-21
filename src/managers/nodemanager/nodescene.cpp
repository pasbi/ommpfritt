#include "managers/nodemanager/nodescene.h"
#include "managers/nodemanager/nodeitem.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "removeif.h"
#include <QTimer>

namespace omm
{

NodeScene::NodeScene(Scene& scene) : scene(scene)
{
  connect(this, &NodeScene::selectionChanged, this, [&scene, this]() {
    if (!m_block_selection_change_notification) {
      scene.set_selection(util::transform<AbstractPropertyOwner*>(selected_nodes()));
    }
  });
  connect(&scene.mail_box(),
          &MailBox::property_value_changed,
          this,
          [this](AbstractPropertyOwner& owner, const QString&, Property&) {
            const auto* const node = kind_cast<nodes::Node*>(&owner);
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
            if (node != nullptr && &node->model() == m_model) {
              QTimer::singleShot(0, this, [this]() { update(); });
            }
          });
}

NodeScene::~NodeScene()
{
  clearSelection();
}

void NodeScene::set_model(nodes::NodeModel* model)
{
  m_block_selection_change_notification = true;
  clear();
  for (auto&& connection : m_scene_model_connections) {
    QObject::disconnect(connection);
  }
  m_scene_model_connections.clear();
  m_model = model;
  if (m_model != nullptr) {
    m_scene_model_connections = {
        connect(m_model, &nodes::NodeModel::node_added, this, [this](auto& node) { add_node(node); }),
        connect(m_model, &nodes::NodeModel::node_removed, this, &NodeScene::remove_node),
        connect(&m_model->compiler(),
                &nodes::AbstractNodeCompiler::compilation_succeeded,
                this,
                [this]() { update(); }),
        connect(&m_model->compiler(),
                &nodes::AbstractNodeCompiler::compilation_failed,
                this,
                [this]() { update(); }),
    };
  }
  if (m_model != nullptr) {
    for (auto* node : model->nodes()) {
      add_node(*node, false);
    }
  }
  m_block_selection_change_notification = false;
}

std::set<nodes::Node*> NodeScene::selected_nodes() const
{
  return util::remove_if(m_model->nodes(), [this](auto* node) {
    return !node_item(*node).isSelected();
  });
}

void NodeScene::add_node(nodes::Node& node, bool select)
{
  auto node_item = std::make_unique<NodeItem>(node);
  auto& node_item_ref = *node_item;
  addItem(node_item.get());
  [[maybe_unused]] const auto [_, success] = m_node_items.insert({&node, std::move(node_item)});
  assert(success);

  if (select) {
    clearSelection();
    node_item_ref.setSelected(true);
  }
}

void NodeScene::remove_node(nodes::Node& node)
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

NodeItem& NodeScene::node_item(nodes::Node& node) const
{
  return *m_node_items.at(&node);
}

}  // namespace omm
