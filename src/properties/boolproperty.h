#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
class BoolProperty : public NumericProperty<bool>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
