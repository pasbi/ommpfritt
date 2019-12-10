#include "managers/nodemanager/nodemanager.h"
#include "logging.h"
#include "managers/nodemanager/nodemodel.h"
#include "ui_nodemanager.h"
#include "mainwindow/application.h"
#include <QMenu>
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
}

NodeManager::~NodeManager()
{
}

QString NodeManager::type() const { return TYPE; }

void NodeManager::set_model(NodeModel* model)
{
  m_ui->nodeview->set_model(model);
}

void NodeManager::contextMenuEvent(QContextMenuEvent* event)
{
  Application& app = Application::instance();
  KeyBindings& kb = app.key_bindings;

  QMenu menu;

  const auto eiff_model_available = [this](auto&& menu) {
    menu->setEnabled(m_ui->nodeview->model() != nullptr);
    return menu;
  };
  const auto eiff_node_selected = [this](auto&& menu) {
    menu->setEnabled(!m_ui->nodeview->selected_nodes().empty());
    return menu;
  };

  menu.addMenu(eiff_model_available(make_add_nodes_menu().release()));
  menu.addAction(eiff_node_selected(kb.make_action(*this, "remove nodes").release()));

  menu.move(mapToGlobal(event->pos()));
  menu.exec();
}

bool NodeManager::perform_action(const QString& name)
{
  if (name == "abort") {
    m_ui->nodeview->abort();
  } else if (name == "remove nodes") {
    m_ui->nodeview->remove_selection();
  } else {
    return false;
  }

  return true;
}

std::unique_ptr<QMenu> NodeManager::make_add_nodes_menu()
{
  auto menu = std::make_unique<QMenu>(tr("Add Node ..."));
  menu->addAction("Hello!");
  return menu;
}

}  // namespace omm
