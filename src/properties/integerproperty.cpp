#include "properties/integerproperty.h"

namespace omm
{

std::string IntegerProperty::type() const
{
  return TYPE;
}

void IntegerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_int(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_int(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void IntegerProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

void IntegerProperty::set(const variant_type& variant)
{
  if (std::holds_alternative<double>(variant)) {
    TypedProperty<int>::set(std::get<double>(variant));
  } else {
    TypedProperty<int>::set(variant);
  }
}

std::unique_ptr<Property> IntegerProperty::clone() const
{
  return std::make_unique<IntegerProperty>(*this);
}

}  // namespace omm
