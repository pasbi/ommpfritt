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
        QObject::tr("Hue"),
        QObject::tr("Saturation"),
        QObject::tr("Value"),
        QObject::tr("Alpha")
      }[channel];
    case Color::Model::Named:
      [[fallthrough]];
    case Color::Model::RGBA:
      return std::vector {
        QObject::tr("Red"),
        QObject::tr("Green"),
        QObject::tr("Blue"),
        QObject::tr("Alpha")
      }[channel];
    default:
      Q_UNREACHABLE();
      return QString();
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
