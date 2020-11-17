#include "properties/floatproperty.h"

namespace omm
{
const Property::PropertyDetail FloatProperty::detail{
    [](const Property&, std::size_t) { return ""; }};

void FloatProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
        deserializer.get_double(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void FloatProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value(default_value(),
                         make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER));
  }
}

}  // namespace omm
