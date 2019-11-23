#include "properties/colorproperty.h"

namespace omm
{

const Property::PropertyDetail ColorProperty::detail
{
  [](const Property& property, std::size_t channel) {
    assert(channel < 4);
    switch (std::get<Color>(property.variant_value()).model()) {
    case Color::Model::HSVA:
      return std::vector {
        tr("Hue"),
        tr("Saturation"),
        tr("Value"),
        tr("Alpha")
      }[channel];
    case Color::Model::Named:
      [[fallthrough]];
    case Color::Model::RGBA:
      return std::vector {
        tr("Red"),
        tr("Green"),
        tr("Blue"),
        tr("Alpha")
      }[channel];
    }
  }
};

void ColorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_color(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
      deserializer.get_color(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void ColorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value( default_value(),
                          make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  }
}

std::unique_ptr<Property> ColorProperty::clone() const
{
  return std::make_unique<ColorProperty>(*this);
}

}  // namespace omm
