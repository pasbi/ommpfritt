#include "managers/manager.h"

namespace omm
{

class MainWindow;

Manager::Manager(const QString& title, Scene& scene)
  : QDockWidget(title)
  , m_scene(scene)
{
  setAttribute(Qt::WA_DeleteOnClose);
}

Scene& Manager::scene() const
{
  return m_scene;
}

}  // namespace omm
