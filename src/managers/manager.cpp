#include "managers/manager.h"
#include <QMenuBar>
#include "mainwindow/application.h"
#include <QContextMenuEvent>

namespace omm
{

class MainWindow;

Manager::Manager(const QString& title, Scene& scene)
  : QDockWidget(title), m_scene(scene)
{
  setAttribute(Qt::WA_DeleteOnClose);
  Application::instance().register_manager(*this);
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

bool Manager::event(QEvent *e)
{
  if (e->type() == QEvent::Polish) {
    for (QWidget* widget : findChildren<QWidget*>()) {
      widget->installEventFilter(this);
    }
  }
  return QDockWidget::event(e);
}

bool Manager::eventFilter(QObject *o, QEvent *e)
{
  if (o->isWidgetType()) {
    auto& w = static_cast<QWidget&>(*o);
    if (isAncestorOf(&w) && e->type() == QEvent::KeyPress) {
      auto& ke = static_cast<QKeyEvent&>(*e);
      if (child_key_press_event(w, ke)) {
        return true;
      }
    }
  }
  return QDockWidget::eventFilter(o, e);
}

bool Manager::child_key_press_event(QWidget &, QKeyEvent &) { return false; }
void Manager::populate_menu(QMenu&) { }
Manager::~Manager()
{
  Application::instance().unregister_manager(*this);
}

Scene& Manager::scene() const { return m_scene; }

void Manager::set_widget(std::unique_ptr<QWidget> widget)
{
  setWidget(widget.release());
}

}  // namespace omm
