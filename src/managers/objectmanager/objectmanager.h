#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeadapter.h"

namespace omm
{

class ObjectTreeView;

class ObjectManager : public ItemManager<ObjectTreeView, ObjectTreeAdapter>
{
  DECLARE_MANAGER_TYPE(ObjectManager)

public:
  explicit ObjectManager(Scene& scene);
};

}  // namespace omm
