#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"

namespace omm
{

class ObjectManager : public ItemManager<ObjectTreeView>
{
  DECLARE_MANAGER_TYPE(ObjectManager)

public:
  explicit ObjectManager(Scene& scene);
};

}  // namespace omm
