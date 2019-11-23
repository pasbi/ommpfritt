#pragma once

#include "properties/typedproperty.h"
#include "color/color.h"
#include <Qt>

namespace omm
{

class ColorProperty : public TypedProperty<Color>
{
public:
  using TypedProperty::TypedProperty;
  QString type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "ColorProperty");
  std::unique_ptr<Property> clone() const override;
  static const PropertyDetail detail;
};

}  // namespace omm
