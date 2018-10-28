#include "managers/objectmanager.h"

#include <glog/logging.h>
#include <QTreeView>
#include <QTimer>

#include "scene/scene.h"

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : Manager(scene)
{
  setWindowTitle(tr("object manager"));
  auto tree_view = std::make_unique<QTreeView>();
  tree_view->setModel(&scene);
  setWidget(tree_view.release());
}

}  // namespace omm
