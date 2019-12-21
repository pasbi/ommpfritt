#include "managers/nodemanager/nodemanager.h"
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
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
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

void NodeManager::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton) {
    auto menu = make_context_menu();
    const QPoint glob_pos = mapToGlobal(event->pos());
    menu->move(glob_pos);
    m_ui->nodeview->populate_context_menu(*menu);
    menu->exec();
  }
  Manager::mousePressEvent(event);
}

void NodeManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!is_locked()) {
    for (AbstractPropertyOwner* apo : selection) {
      if (!!(apo->flags() & AbstractPropertyOwner::Flag::HasNodes)) {
        NodeModel& nodes_model = dynamic_cast<NodesOwner*>(apo)->node_model();
        for (NodeManager* nm : Application::instance().managers<NodeManager>()) {
          nm->set_model(&nodes_model);
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

  menu->addMenu(eiff_model_available(make_add_nodes_menu(kb).release()));
  menu->addAction(eiff_node_selected(kb.make_action(*this, "remove nodes").release()));

  return menu;
}

bool NodeManager::perform_action(const QString& name)
{
  if (name == "abort") {
    m_ui->nodeview->abort();
  } else if (name == "remove nodes") {
    m_ui->nodeview->remove_selection();
  } else if (::contains(Node::keys(), name)) {
    std::vector<std::unique_ptr<Node>> nodes;
    auto node = Node::make(name, &scene());
    const QSizeF size = m_ui->nodeview->node_geometry(*node).size();
    node->set_pos(m_ui->nodeview->get_insert_position() - QPointF(size.width(), size.height()) / 2.0);
    nodes.push_back(std::move(node));
    scene().submit<AddNodesCommand>(*m_ui->nodeview->model(), std::move(nodes));
  } else {
    return false;
  }

  return true;
}

std::unique_ptr<QMenu> NodeManager::make_add_nodes_menu(KeyBindings& kb)
{
  auto menu = std::make_unique<QMenu>(tr("Add Node ..."));
  for (const QString& name : Node::keys()) {
    const QString tr_name = QCoreApplication::translate("any-context", name.toStdString().c_str());
    auto action = kb.make_action(*this, name);
    menu->addAction(action.release());
  }
  return menu;
}

}  // namespace omm
