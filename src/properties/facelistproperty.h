#pragma once

#include "properties/typedproperty.h"
#include "facelist.h"

namespace omm
{

class FaceListProperty : public TypedProperty<FaceList>
{
public:
  using TypedProperty::TypedProperty;
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;

  static const PropertyDetail detail;
};

}  // namespace omm
