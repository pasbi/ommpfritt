#include "properties/floatproperty.h"

namespace omm
{

std::string FloatProperty::type() const
{
  return "FloatProperty";
}

std::string FloatProperty::widget_type() const
{
  return "FloatPropertyWidget";
}

void FloatProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Property::deserialize(deserializer, root);
  set_value(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_double(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

}  // namespace omm
