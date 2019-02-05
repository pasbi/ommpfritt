#pragma once

#include "properties/numericproperty.h"

namespace omm
{

class FloatProperty : public NumericProperty<double>
{
public:
  using NumericProperty<double>::NumericProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "FloatProperty";
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
