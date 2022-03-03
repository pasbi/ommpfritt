#include "managers/nodemanager/nodemanager.h"
#include "commands/nodecommand.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "main/application.h"
#include "managers/nodemanager/nodemanagertitlebar.h"
#include "managers/nodemanager/nodescene.h"
#include "nodesystem/node.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodesowner.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "ui_nodemanager.h"
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMimeData>

namespace
{

using namespace omm;

bool accept_node(const nodes::NodeModel& model, const QString& name)
{
  return nodes::Node::detail(name).definitions.contains(model.language());
}

QMenu& find_menu(QMenu* root, std::map<QString, QMenu*>& sub_menus, const std::vector<const char*>& path)
{
  for (const char* token : path) {
    auto it = sub_menus.find(token);
    if (it == sub_menus.end()) {
      const QString tr_menu_name = QApplication::translate("NodeMenuPath", token);
      auto menu = std::make_unique<QMenu>(tr_menu_name);
      QMenu& ref = *menu;
      sub_menus.insert({token, &ref});
      root->addMenu(menu.release());
      root = &ref;
    } else {
      root = it->second;
    }
  }
  return *root;
}

}  // namespace

namespace omm
{
NodeManager::NodeManager(Scene& scene)
    : Manager(tr("Nodes"), scene), m_ui(std::make_unique<Ui::NodeManager>())
{
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  connect(m_ui->nodeview, &NodeView::customContextMenuRequested, this, [this](const QPoint& pos) {
    auto menu = make_context_menu();
    const QPoint glob_pos = m_ui->nodeview->mapToGlobal(pos);
    menu->move(glob_pos);
    m_ui->nodeview->populate_context_menu(*menu);
    menu->exec();
  });
  widget->setContextMenuPolicy(Qt::NoContextMenu);
  set_widget(std::move(widget));

  connect(&scene.mail_box(), &MailBox::selection_changed, this, &NodeManager::set_selection);
  connect(&scene.mail_box(), &MailBox::abstract_property_owner_removed, this, [this](const auto& apo) {
    const auto* nodes_owner = dynamic_cast<const nodes::NodesOwner*>(&apo);
    if (nodes_owner != nullptr && &nodes_owner->node_model() == m_ui->nodeview->model()) {
      set_model(nullptr);
    }
  });
  connect(&scene.mail_box(), &MailBox::about_to_reset, this, [this] { set_model(nullptr); });
  setTitleBarWidget(std::make_unique<NodeManagerTitleBar>(*this).release());
}

NodeManager::~NodeManager() = default;

QString NodeManager::type() const
{
  return TYPE;
}

void NodeManager::set_model(nodes::NodeModel* model)
{
  if (NodeScene* scene = m_ui->nodeview->scene(); scene != nullptr) {
    QSignalBlocker blocker(scene);
    scene->clearSelection();
  }
  m_ui->nodeview->set_model(model);
}

void NodeManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!is_locked()) {
    for (AbstractPropertyOwner* apo : selection) {
      if (!!(apo->flags() & Flag::HasNodes)) {
        auto& model = dynamic_cast<const nodes::NodesOwner*>(apo)->node_model();
        if (m_ui->nodeview->model() != &model) {
          set_model(&model);
        }
      }
    }
  }
}

std::unique_ptr<QMenu> NodeManager::make_context_menu()
{
  Application& app = Application::instance();
  KeyBindings& kb = *app.key_bindings;

  auto menu = std::make_unique<QMenu>();

  const auto eiff_model_available = [this](auto&& menu) {
    menu->setEnabled(m_ui->nodeview->model() != nullptr);
    return menu;
  };
  const auto eiff_node_selected = [this](auto&& menu) {
    menu->setEnabled(!m_ui->nodeview->selected_nodes().empty());
    return menu;
  };
  const auto eiff_clipboard_has_nodes = [this](auto&& menu) {
    menu->setEnabled(m_ui->nodeview->accepts_paste(*QApplication::clipboard()->mimeData()));
    return menu;
  };

  menu->addMenu(eiff_model_available(make_add_nodes_menu(kb).release()));
  menu->addAction(eiff_node_selected(kb.make_menu_action(*this, "remove nodes").release()));
  menu->addSeparator();
  menu->addAction(eiff_node_selected(kb.make_menu_action(*this, "copy").release()));
  menu->addAction(eiff_clipboard_has_nodes(kb.make_menu_action(*this, "paste").release()));
  menu->addAction(eiff_node_selected(kb.make_menu_action(*this, "cut").release()));

  return menu;
}

bool NodeManager::perform_action(const QString& name)
{
  if (name == "abort") {
    m_ui->nodeview->abort();
  } else if (name == "remove nodes") {
    m_ui->nodeview->remove_selection();
  } else if (name == "copy") {
    m_ui->nodeview->copy_to_clipboard();
  } else if (name == "cut") {
    m_ui->nodeview->copy_to_clipboard();
    m_ui->nodeview->remove_selection();
  } else if (name == "paste") {
    m_ui->nodeview->paste_from_clipboard();
  } else if (nodes::Node::keys().contains(name)) {
    std::vector<std::unique_ptr<nodes::Node>> nodes;
    auto& model = *m_ui->nodeview->model();
    auto node = nodes::Node::make(name, model);
    node->set_pos(m_ui->nodeview->node_insert_pos());
    nodes.push_back(std::move(node));
    scene().submit<AddNodesCommand>(model, std::move(nodes));
  } else {
    return false;
  }

  return true;
}

std::unique_ptr<QMenu> NodeManager::create_quick_constant_node_actions_menu(omm::nodes::NodeModel& model) const
{
  auto quick_constant_node_actions_menu = std::make_unique<QMenu>(tr("Constant ..."));
  const auto types = model.compiler().supported_types();

  for (const Type& type : types) {
    const QString label = QApplication::translate("Property", variant_type_name(type).data());
    auto action = std::make_unique<QAction>(label);
    connect(action.get(), &QAction::triggered, &model, [type, &model, label, this]() {
      auto node = std::make_unique<nodes::ConstantNode>(model);
      const auto property_name = Property::property_type(type);
      auto property = Property::make(property_name);
      property->set_category(Property::USER_PROPERTY_CATEGROY_NAME);
      property->set_label(label);
      node->add_property(property_name, std::move(property));
      std::vector<std::unique_ptr<nodes::Node>> nodes;
      node->set_pos(m_ui->nodeview->node_insert_pos());
      nodes.push_back(std::move(node));
      scene().submit<AddNodesCommand>(model, std::move(nodes));
    });
    quick_constant_node_actions_menu->addAction(action.release());
  }
  return quick_constant_node_actions_menu;
}

void NodeManager::create_node_actions(const omm::nodes::NodeModel& model,
                                      const KeyBindings& kb,
                                      QMenu& root_menu,
                                      std::map<QString, QMenu*>& sub_menus)
{
  for (const QString& name : nodes::Node::keys()) {
    if (accept_node(model, name)) {
      if (const auto menu_path = nodes::Node::detail(name).menu_path; !menu_path.empty()) {
        auto action = kb.make_menu_action(*this, name);
        QMenu& menu = find_menu(&root_menu, sub_menus, menu_path);
        menu.addAction(action.release());
      }
    }
  }
}

std::unique_ptr<QMenu> NodeManager::make_add_nodes_menu(KeyBindings& kb)
{
  auto root_menu = std::make_unique<QMenu>(tr("Add Node ..."));
  if (auto* const model = m_ui->nodeview->model(); model != nullptr) {
    std::map<QString, QMenu*> sub_menus;
    create_node_actions(*model, kb, *root_menu, sub_menus);
    root_menu->addMenu(create_quick_constant_node_actions_menu(*model).release());
  }
  return root_menu;
}

}  // namespace omm
