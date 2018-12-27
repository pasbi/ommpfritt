#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class IntegerProperty : public TypedProperty<int>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void set(const variant_type& variant) override;
  static constexpr auto TYPE = "IntegerProperty";
};

}  // namespace omm
