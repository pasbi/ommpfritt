#include "managers/manager.h"
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

Manager::~Manager()
{
  Application::instance().unregister_manager(*this);
}

Scene& Manager::scene() const { return m_scene; }

void Manager::set_widget(std::unique_ptr<QWidget> widget)
{
  setWidget(widget.release());
}

void Manager::keyPressEvent(QKeyEvent* e)
{
  if (!::contains(Application::keyboard_modifiers, e->key()) && isFloating()) {
    // MainWindow is not a parent, hence the manager must handle the key event itself.
    if (Application::instance().dispatch_key(e->key(), e->modifiers(), *this)) {
      QDockWidget::keyPressEvent(e);
    } else {
      raise();
    }
  } else {
    // Let the MainWindow handle the event and dispatch the keybinding.
    QDockWidget::keyPressEvent(e);
  }
}

}  // namespace omm
