#pragma once

#include "properties/typedproperty.h"
#include "splinetype.h"
#include <Qt>

namespace omm
{
class SplineProperty : public TypedProperty<SplineType>
{
public:
  explicit SplineProperty(const SplineType& default_value = SplineType());
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static constexpr auto MODE_PROPERTY_KEY = "mode";

  static const PropertyDetail detail;
};

}  // namespace omm
