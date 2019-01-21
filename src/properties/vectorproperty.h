#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class FloatVectorProperty : public TypedProperty<VectorPropertyValueType<arma::vec2>>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "FloatVectorProperty";
  std::unique_ptr<Property> clone() const override;
};

class IntegerVectorProperty : public TypedProperty<VectorPropertyValueType<arma::ivec2>>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "IntegerVectorProperty";
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
