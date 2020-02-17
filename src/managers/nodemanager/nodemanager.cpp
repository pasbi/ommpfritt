#include "managers/nodemanager/nodemanager.h"
#include "managers/nodemanager/nodes/constantnode.h"
#include "managers/nodemanager/nodecompiler.h"
#include <QMimeData>
#include <QClipboard>
#include "managers/nodemanager/nodemanagertitlebar.h"
#include "managers/nodemanager/nodesowner.h"
#include "commands/nodecommand.h"
#include "managers/nodemanager/node.h"
#include "logging.h"
#include "managers/nodemanager/nodemodel.h"
#include "ui_nodemanager.h"
#include "mainwindow/application.h"
#include "keybindings/menu.h"
#include "scene/messagebox.h"
#include <QContextMenuEvent>

namespace omm
{

NodeManager::NodeManager(Scene& scene)
  : Manager(tr("Nodes"), scene)
  , m_ui(std::make_unique<Ui::NodeManager>())
{
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  connect(m_ui->nodeview, &NodeView::customContextMenuRequested, [this](const QPoint& pos) {
    auto menu = make_context_menu();
    const QPoint glob_pos = m_ui->nodeview->mapToGlobal(pos);
    menu->move(glob_pos);
    m_ui->nodeview->populate_context_menu(*menu);
    menu->exec();
  });
  set_widget(std::move(widget));
  setContextMenuPolicy(Qt::PreventContextMenu);  // we implement it ourself.

  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
  setTitleBarWidget(std::make_unique<NodeManagerTitleBar>(*this).release());
}

NodeManager::~NodeManager()
{
}

QString NodeManager::type() const { return TYPE; }

void NodeManager::set_model(NodeModel* model)
{
  m_ui->nodeview->set_model(model);
}

void NodeManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!is_locked()) {
    for (AbstractPropertyOwner* apo : selection) {
      if (!!(apo->flags() & Flag::HasNodes)) {
        NodeModel& nodes_model = dynamic_cast<AbstractNodesOwner*>(apo)->node_model();
        for (Manager* nm : Application::instance().managers(NodeManager::TYPE)) {
          static_cast<NodeManager*>(nm)->set_model(&nodes_model);
        }
      }
    }
  }
}

std::unique_ptr<QMenu> NodeManager::make_context_menu()
{
  Application& app = Application::instance();
  KeyBindings& kb = app.key_bindings;

  auto menu = std::make_unique<Menu>();

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
  menu->addAction(eiff_node_selected(kb.make_menu_action(*this, "copy").release()));
  menu->addAction(eiff_clipboard_has_nodes(kb.make_menu_action(*this, "paste").release()));

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
  } else if (name == "paste") {
    m_ui->nodeview->paste_from_clipboard();
  } else if (::contains(Node::keys(), name)) {
    std::vector<std::unique_ptr<Node>> nodes;
    auto& model = *m_ui->nodeview->model();
    auto node = Node::make(name, model);
    const QPointF gpos = m_ui->nodeview->mapFromGlobal(QCursor::pos());
    node->set_pos(m_ui->nodeview->transform().map(gpos));
    nodes.push_back(std::move(node));
    scene().submit<AddNodesCommand>(model, std::move(nodes));
  } else {
    return false;
  }

  return true;
}

std::unique_ptr<QMenu> NodeManager::make_add_nodes_menu(KeyBindings& kb)
{
  auto menu = std::make_unique<QMenu>(tr("Add Node ..."));
  if (NodeModel* model = m_ui->nodeview->model(); model != nullptr) {
    const auto language = model->language();
    for (const QString& name : Node::keys()) {
      if (::contains(Node::detail(name).definitions, language)) {
        const QString tr_name = QCoreApplication::translate("any-context", name.toStdString().c_str());
        auto action = kb.make_menu_action(*this, name);
        menu->addAction(action.release());
      }
    }

    {
      auto quick_constant_node_actions_menu = std::make_unique<QMenu>(tr("Constant ..."));
      const auto types = AbstractNodeCompiler::supported_types(model->language());
      for (const QString& type : types) {
        const QString label = QApplication::translate("Property", type.toStdString().c_str());
        auto action = std::make_unique<QAction>(label);
        connect(action.get(), &QAction::triggered, [type, model, label, this]() {
          auto node = std::make_unique<ConstantNode>(*model);
          auto property = Property::make(type);
          property->set_category(Property::USER_PROPERTY_CATEGROY_NAME);
          property->set_label(label);
          node->add_property(type, std::move(property));
          std::vector<std::unique_ptr<Node>> nodes;
          nodes.push_back(std::move(node));
          scene().submit<AddNodesCommand>(*model, std::move(nodes));
        });
        quick_constant_node_actions_menu->addAction(action.release());
      }
      menu->addMenu(quick_constant_node_actions_menu.release());
    }
  }
  return menu;
}

}  // namespace omm
