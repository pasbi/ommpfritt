#pragma once

#include "properties/typedproperty.h"
#include "facelist.h"

namespace omm
{

class FaceListProperty : public TypedProperty<FaceList>
{
public:
  explicit FaceListProperty(const FaceList& default_value = FaceList{});
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  static constexpr auto MODE_PROPERTY_KEY = "mode";

  static const PropertyDetail detail;
};

}  // namespace omm
