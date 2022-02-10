#include "managers/manager.h"
#include "main/application.h"
#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

namespace omm
{
class MainWindow;

Manager::Manager(const QString& title, Scene& scene) : QDockWidget(title), m_scene(scene)
{
  setAttribute(Qt::WA_DeleteOnClose);
  Application::instance().register_manager(*this);
}

Manager::~Manager()
{
  Application::instance().unregister_manager(*this);
}

Scene& Manager::scene() const
{
  return m_scene;
}

bool Manager::is_visible() const
{
  if (!isVisible()) {
    return false;
  } else {
    QPoint parent_top_left(0, 0);
    if (isWindow()) {
      const auto* screen = [this]() -> const QScreen* {
        const QWidget* npw = nativeParentWidget();
        if (npw == nullptr) {
          return nullptr;
        }
        const QWindow* window = npw->windowHandle();
        if (window == nullptr) {
          return nullptr;
        }
        return window->screen();
      }();

      if (screen == nullptr) {
        parent_top_left = QGuiApplication::primaryScreen()->availableVirtualGeometry().topLeft();
      }
    }

    const QPoint bottom_right = geometry().bottomRight();
    return bottom_right.x() >= parent_top_left.x() && bottom_right.y() >= parent_top_left.y();
  }
}

void Manager::set_widget(std::unique_ptr<QWidget> widget)
{
  setWidget(widget.release());
}

void Manager::keyPressEvent(QKeyEvent* e)
{
  if (!Application::keyboard_modifiers.contains(e->key()) && isFloating()) {
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

bool Manager::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride) {
    // QEvent documentation: "If the shortcut override is accepted, the event is delivered
    // as a normal key press to the focus widget."
    event->accept();
  }
  return QDockWidget::event(event);
}

}  // namespace omm
