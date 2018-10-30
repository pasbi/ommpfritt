#pragma once

#include "managers/manager.h"
#include <glog/logging.h>

#include "managers/objectmanager/objecttreeadapter.h"

namespace omm
{

class ObjectManager : public Manager
{
  Q_OBJECT
  DECLARE_MANAGER_TYPE(ObjectManager)

public:
  explicit ObjectManager(omm::Scene& scene);
  ~ObjectManager();

private:
  ObjectTreeAdapter m_object_tree_adapter;

  void on_selection_changed(const QItemSelection& selection, const QItemSelection& old_selection);
};

}  // namespace omm
