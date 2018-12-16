#include "properties/boolproperty.h"

namespace omm
{

std::string BoolProperty::type() const
{
  return "BoolProperty";
}

std::string BoolProperty::widget_type() const
{
  return "BoolPropertyWidget";
}

void BoolProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_bool(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_bool(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void BoolProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

}  // namespace omm
