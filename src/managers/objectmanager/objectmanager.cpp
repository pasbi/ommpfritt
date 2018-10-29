#include "managers/objectmanager/objectmanager.h"

#include <glog/logging.h>
#include <QTreeView>
#include <QTimer>

#include "scene/scene.h"

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : Manager(tr("Objects"), scene)
  , m_object_tree_adapter(scene.root())
{
  setWindowTitle(tr("object manager"));

  scene.register_object_tree_adapter(m_object_tree_adapter);

  auto tree_view = std::make_unique<QTreeView>();
  tree_view->setModel(&m_object_tree_adapter);

  setWidget(tree_view.release());
  setObjectName(TYPE());
}

ObjectManager::~ObjectManager()
{
  m_scene.unregister_object_tree_adapter(m_object_tree_adapter);
}

}  // namespace omm
