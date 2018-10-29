#pragma once

#include <vector>
#include <functional>

#include "properties/hasproperties.h"
#include "managers/manager.h"

namespace omm
{

class PropertyView;

class PropertyManager : public omm::Manager
{
  Q_OBJECT
  DECLARE_MANAGER_TYPE(PropertyManager)

public:
  explicit PropertyManager(omm::Scene& scene);

  void select(const std::vector<std::reference_wrapper<HasProperties>>& selection);
  std::vector<HasProperties::PropertyKey> keys();

  PropertyView property(const std::string& key);
};

}  // namespace omm
