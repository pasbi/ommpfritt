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
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static const PropertyDetail detail;
};

}  // namespace omm
