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
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
