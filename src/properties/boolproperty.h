#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
class BoolProperty : public NumericProperty<bool>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  static const PropertyDetail detail;
};

}  // namespace omm
