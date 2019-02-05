#pragma once

#include "properties/numericproperty.h"

namespace omm
{

class IntegerProperty : public NumericProperty<int>
{
public:
  using NumericProperty<int>::NumericProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "IntegerProperty";
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
