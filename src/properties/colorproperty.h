#pragma once

#include "color/color.h"
#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{
class ColorProperty : public TypedProperty<Color>
{
public:
  using TypedProperty::TypedProperty;
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  static const PropertyDetail detail;
};

}  // namespace omm
