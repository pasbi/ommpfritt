#include "properties/splineproperty.h"

namespace omm
{

const Property::PropertyDetail SplineProperty::detail { nullptr };

SplineProperty::SplineProperty(const omm::SplineType& default_value)
{

}

void SplineProperty::deserialize(omm::AbstractDeserializer& deserializer,
                                 const omm::Serializable::Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
}

void SplineProperty::serialize(omm::AbstractSerializer& serializer,
                               const omm::Serializable::Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
}

}  // namespace omm
