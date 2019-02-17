#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"

#include <glog/logging.h>

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager(tr("Objects"), scene, scene.object_tree_adapter)
{
  setWindowTitle(tr("object manager"));
  setObjectName(TYPE);
}

}  // namespace omm
