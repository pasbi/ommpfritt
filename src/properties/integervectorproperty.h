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
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  static const PropertyDetail detail;
};

}  // namespace omm
