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
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("ColorProperty", "ColorProperty");
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
