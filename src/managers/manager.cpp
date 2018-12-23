#include "managers/manager.h"
#include <QMenuBar>
#include <glog/logging.h>

namespace omm
{

class MainWindow;

Manager::Manager(const QString& title, Scene& scene, std::unique_ptr<QMenuBar> menu_bar)
  : QDockWidget(title)
  , m_scene(scene)
{
  setAttribute(Qt::WA_DeleteOnClose);

  setWidget(std::make_unique<QWidget>(this).release());
  widget()->setLayout(std::make_unique<QVBoxLayout>(widget()).release());

  if (menu_bar) {
    widget()->layout()->addWidget(menu_bar.release());
  }
}

Scene& Manager::scene() const
{
  return m_scene;
}

void Manager::set_widget(std::unique_ptr<QWidget> widget)
{
  this->widget()->layout()->addWidget(widget.release());
}


}  // namespace omm
