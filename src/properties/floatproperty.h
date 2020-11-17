#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
class FloatProperty : public NumericProperty<double>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
