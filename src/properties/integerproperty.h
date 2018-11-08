#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class IntegerProperty : public TypedProperty<int>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  std::string widget_type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
};

}  // namespace omm
