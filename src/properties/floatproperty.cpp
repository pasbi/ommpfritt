#include "properties/floatproperty.h"

namespace omm
{

std::string FloatProperty::type() const
{
  return TYPE;
}

void FloatProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_double(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void FloatProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

void FloatProperty::set(const variant_type& variant)
{
  if (std::holds_alternative<int>(variant)) {
    TypedProperty<double>::set(std::get<int>(variant));
  } else {
    TypedProperty<double>::set(variant);
  }
}

std::unique_ptr<Property> FloatProperty::clone() const
{
  return std::make_unique<FloatProperty>(*this);
}

}  // namespace omm
