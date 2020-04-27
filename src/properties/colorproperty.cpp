#include "properties/colorproperty.h"

namespace omm
{

const Property::PropertyDetail ColorProperty::detail
{
  [](const Property& property, std::size_t channel) -> QString {
    const auto color = std::get<Color>(property.variant_value());
    const auto name = Color::component_names.at(color.model())[channel];
    return QCoreApplication::translate("Color", name.toStdString().c_str());
  }
};

void ColorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_color(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    const auto ptr = make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER);
    set_default_value(deserializer.get_color(ptr));
  }
}

void ColorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    const auto ptr = make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER);
    serializer.set_value(default_value(), ptr);
  }
}

}  // namespace omm
