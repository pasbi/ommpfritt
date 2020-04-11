#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{

class IntegerProperty : public NumericProperty<int>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
