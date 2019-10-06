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
