#pragma once

#include "managers/manager.h"
#include <glog/logging.h>

namespace omm
{

class ObjectManager : public Manager
{
  Q_OBJECT
public:
  explicit ObjectManager(omm::Scene& scene);

public Q_SLOTS:
  void print_begin() { ; }
  void print_end() { ; }

};

}  // namespace omm
