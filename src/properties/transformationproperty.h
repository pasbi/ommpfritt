#pragma once

#include "properties/typedproperty.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class TransformationProperty : public TypedProperty<ObjectTransformation>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "TransformationProperty";
};

}  // namespace omm
