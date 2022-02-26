#include "properties/facesproperty.h"


namespace omm
{

const Property::PropertyDetail FacesProperty::detail{nullptr};

void FacesProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_faces(root / "value"));
}

void FacesProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
}


}  // namespace omm
