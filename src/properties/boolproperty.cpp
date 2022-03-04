#include "properties/boolproperty.h"

namespace omm
{
const Property::PropertyDetail BoolProperty::detail{
    [](const Property&, std::size_t) { return ""; }};

void BoolProperty::deserialize(serialization::DeserializerWorker& worker)
{
  // intentionally skip NumericProperty::deserialize.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_bool());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get_bool());
  }
}

void BoolProperty::serialize(serialization::SerializerWorker& worker) const
{
  // intentionally skip NumericProperty::serialize.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

}  // namespace omm
