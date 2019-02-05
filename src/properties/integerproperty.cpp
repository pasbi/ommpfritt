#include "properties/integerproperty.h"

namespace omm
{

std::string IntegerProperty::type() const
{
  return TYPE;
}

void IntegerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_int(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_int(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void IntegerProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> IntegerProperty::clone() const
{
  return std::make_unique<IntegerProperty>(*this);
}

}  // namespace omm
