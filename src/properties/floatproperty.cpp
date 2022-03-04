#include "properties/floatproperty.h"

namespace omm
{
const Property::PropertyDetail FloatProperty::detail{
    [](const Property&, std::size_t) { return ""; }};

void FloatProperty::deserialize(serialization::DeserializerWorker& worker)
{
  NumericProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_double());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get_double());
  }
}

void FloatProperty::serialize(serialization::SerializerWorker& worker) const
{
  NumericProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

}  // namespace omm
