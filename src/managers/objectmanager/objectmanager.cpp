#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"

#include <glog/logging.h>

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager(tr("Objects"), scene)
{
  setWindowTitle(tr("object manager"));
  setObjectName(TYPE());
}

}  // namespace omm
