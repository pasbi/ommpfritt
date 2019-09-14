#include "properties/floatproperty.h"

namespace omm
{

void FloatProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_double(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void FloatProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> FloatProperty::clone() const
{
  return std::make_unique<FloatProperty>(*this);
}

}  // namespace omm
