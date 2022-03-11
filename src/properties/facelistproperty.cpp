#include "properties/facelistproperty.h"
#include <QCoreApplication>

namespace omm
{

const Property::PropertyDetail FaceListProperty::detail{nullptr};

FaceListProperty::FaceListProperty(const FaceList& default_value)
    : TypedProperty(default_value)
{
}

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
