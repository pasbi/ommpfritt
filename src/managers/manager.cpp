#include "managers/manager.h"
#include <QMenuBar>
#include <glog/logging.h>
#include "mainwindow/application.h"
#include <QContextMenuEvent>

namespace omm
{

class MainWindow;

Manager::Manager(const QString& title, Scene& scene, std::unique_ptr<QMenuBar> menu_bar)
  : QDockWidget(title), m_scene(scene)
{
  setAttribute(Qt::WA_DeleteOnClose);

  setWidget(std::make_unique<QWidget>(this).release());
  widget()->setLayout(std::make_unique<QVBoxLayout>(widget()).release());

  if (menu_bar) { widget()->layout()->addWidget(menu_bar.release()); }
  m_scene.Observed<AbstractSelectionObserver>::register_observer(*this);
}

Manager::~Manager()
{
  m_scene.Observed<AbstractSelectionObserver>::unregister_observer(*this);
}

void Manager::contextMenuEvent(QContextMenuEvent *event)
{
  auto& app = Application::instance();
  auto menus = app.key_bindings.make_menus(app, application_actions());
  std::unique_ptr<QMenu> menu;
  if (menus.size() > 0) {
    assert(menus.size() == 1);
    menu = std::move(menus.front());
  } else {
    menu = std::make_unique<QMenu>();
  }

  populate_menu(*menu);
  if (menu->actions().size() > 0) {
    menu->move(event->globalPos());
    menu->exec();

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu.release();
    event->accept();
  }
}

std::vector<std::string> Manager::application_actions() const { return {}; }
void Manager::populate_menu(QMenu&) { }
Scene& Manager::scene() const { return m_scene; }

void Manager::set_widget(std::unique_ptr<QWidget> widget)
{
  this->widget()->layout()->addWidget(widget.release());
}

}  // namespace omm
