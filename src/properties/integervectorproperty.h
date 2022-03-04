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
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
