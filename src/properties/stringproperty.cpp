#include "properties/stringproperty.h"

namespace omm
{

std::string StringProperty::type() const
{
  return "StringProperty";
}

std::string StringProperty::widget_type() const
{
  return "StringPropertyWidget";
}

void StringProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set_value(deserializer.get_string(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_string(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void StringProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

}  // namespace omm
