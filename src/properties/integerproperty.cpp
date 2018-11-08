#include "properties/integerproperty.h"

namespace omm
{

std::string IntegerProperty::type() const
{
  return "IntegerProperty";
}

std::string IntegerProperty::widget_type() const
{
  return "IntegerPropertyWidget";
}

void IntegerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Property::deserialize(deserializer, root);
  set_value(deserializer.get_int(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_int(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

}  // namespace omm
