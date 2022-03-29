#include "properties/splineproperty.h"

namespace omm
{
const Property::PropertyDetail SplineProperty::detail{nullptr};

void SplineProperty::deserialize(serialization::DeserializerWorker& worker)
{
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get<SplineType>());
}

void SplineProperty::serialize(serialization::SerializerWorker& worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
}

}  // namespace omm
