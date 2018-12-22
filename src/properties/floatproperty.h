#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class FloatProperty : public TypedProperty<double>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  std::string widget_type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void set(const variant_type& variant) override;
};

}  // namespace omm
