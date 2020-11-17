#include "properties/splineproperty.h"

namespace omm
{
const Property::PropertyDetail SplineProperty::detail{nullptr};

SplineProperty::SplineProperty(const omm::SplineType& default_value)
    : TypedProperty<SplineType>(default_value)
{
}

void SplineProperty::deserialize(omm::AbstractDeserializer& deserializer,
                                 const omm::Serializable::Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_spline(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
}

void SplineProperty::serialize(omm::AbstractSerializer& serializer,
                               const omm::Serializable::Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
}

}  // namespace omm
