#pragma once

#include "managers/manager.h"
#include <glog/logging.h>

#include "managers/objectmanager/objecttreeadapter.h"

namespace omm
{

class ObjectManager : public Manager
{
  Q_OBJECT
public:
  explicit ObjectManager(omm::Scene& scene);
  ~ObjectManager();

private:
  ObjectTreeAdapter m_object_tree_adapter;

};

}  // namespace omm
