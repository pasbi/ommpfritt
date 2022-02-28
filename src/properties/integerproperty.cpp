#include "properties/integerproperty.h"

namespace omm
{
const Property::PropertyDetail IntegerProperty::detail{
    [](const Property&, std::size_t) { return ""; }};

void IntegerProperty::deserialize(serialization::DeserializerWorker& worker)
{
  NumericProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_int());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get_int());
  }
}

void IntegerProperty::serialize(serialization::SerializerWorker& worker) const
{
  NumericProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

IntegerProperty& IntegerProperty::set_special_value(const QString& label)
{
  special_value_label = label;
  return *this;
}

}  // namespace omm
