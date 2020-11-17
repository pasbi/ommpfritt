#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
struct IntegerVectorPropertyLimits {
  static const Vec2i lower;
  static const Vec2i upper;
  static const Vec2i step;
};

class IntegerVectorProperty : public NumericProperty<Vec2i>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
