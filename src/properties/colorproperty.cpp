#include "properties/colorproperty.h"

namespace omm
{

std::string ColorProperty::type() const
{
  return TYPE;
}

void ColorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_color(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_color(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void ColorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

}  // namespace omm
