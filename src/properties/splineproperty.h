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
  QString type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "SplineProperty");
  static constexpr auto MODE_PROPERTY_KEY = "mode";

  static const PropertyDetail detail;
};

}  // namespace omm
