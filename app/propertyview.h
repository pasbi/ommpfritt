#pragma once

#include <vector>
#include <functional>

namespace omm {

class PropertyView
{
  PropertyView(const std::vector<std::reference_wrapper<Property>> properties);

};

}  // namespace omm
