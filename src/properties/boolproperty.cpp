#include "properties/boolproperty.h"

namespace omm
{
const Property::PropertyDetail BoolProperty::detail{
    [](const Property&, std::size_t) { return ""; }};

void BoolProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  // intentionally skip NumericProperty::deserialize.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_bool(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
        deserializer.get_bool(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void BoolProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  // intentionally skip NumericProperty::serialize.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value(default_value(),
                         make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER));
  }
}

}  // namespace omm
