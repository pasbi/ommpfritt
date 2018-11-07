#pragma once

#include "orderedmap.h"

namespace omm
{

class Property;
using PropertyMap = OrderedMap<std::string, std::unique_ptr<Property>>;

}
