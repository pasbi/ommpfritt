#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
class FloatProperty : public NumericProperty<double>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  static const PropertyDetail detail;
};

}  // namespace omm
