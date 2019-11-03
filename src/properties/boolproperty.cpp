#include "properties/boolproperty.h"

namespace omm
{

void BoolProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_bool(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
      deserializer.get_bool(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void BoolProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value( default_value(),
                          make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  }
}

std::unique_ptr<Property> BoolProperty::clone() const
{
  return std::make_unique<BoolProperty>(*this);
}

}  // namespace omm
