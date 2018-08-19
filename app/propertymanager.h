#pragma once

#include "hasproperties.h"
#include <vector>
#include <functional>

class PropertyView;

class PropertyManager
{
public:
  PropertyManager();

  void select(const std::vector<std::reference_wrapper<HasProperties>>& selection);
  std::vector<HasProperties::PropertyKey> keys();

  PropertyView property(const std::string& key);
};