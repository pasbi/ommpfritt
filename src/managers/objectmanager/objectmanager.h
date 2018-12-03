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

protected:
  void on_selection_changed( const QItemSelection& selection,
                             const QItemSelection& old_selection ) override;
};

}  // namespace omm
