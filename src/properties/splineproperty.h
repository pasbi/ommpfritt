#pragma once

#include "properties/typedproperty.h"
#include "splinetype.h"

namespace omm
{

class SplineProperty : public TypedProperty<SplineType>
{
public:
  using TypedProperty::TypedProperty;
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static constexpr auto MODE_PROPERTY_KEY = "mode";

  static const PropertyDetail detail;
};

}  // namespace omm
