#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "managers/objectmanager/objecttreeview.h"

namespace omm
{

class ObjectManager : public ItemManager<ObjectTreeView, ObjectTreeAdapter>
{
  DECLARE_MANAGER_TYPE(ObjectManager)

public:
  explicit ObjectManager(Scene& scene);
};

}  // namespace omm
