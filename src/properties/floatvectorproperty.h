#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
struct FloatVectorPropertyLimits {
  static const Vec2f lower;
  static const Vec2f upper;
  static const Vec2f step;
};

class FloatVectorProperty : public NumericProperty<Vec2f>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
