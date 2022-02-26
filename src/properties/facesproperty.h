#pragma once

#include "path/face.h"
#include "properties/typedproperty.h"


namespace omm
{

using Faces = std::deque<Face>;
class FacesProperty : public TypedProperty<Faces>
{
public:
  using TypedProperty::TypedProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
