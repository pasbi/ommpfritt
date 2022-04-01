#include "properties/facelistproperty.h"

namespace omm
{

const Property::PropertyDetail FaceListProperty::detail{nullptr};

void FaceListProperty::deserialize(serialization::DeserializerWorker &worker)
{
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get<FaceList>());
}

void FaceListProperty::serialize(serialization::SerializerWorker &worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
}

}  // namespace omm
