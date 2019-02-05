#pragma once

#include "properties/numericproperty.h"

namespace omm
{

struct IntegerPropertyLimits
{
  static constexpr auto upper = std::numeric_limits<int>::max();
  static constexpr auto lower = std::numeric_limits<int>::lowest();
  static constexpr auto step = 1;
};

class IntegerProperty : public NumericProperty<int, IntegerPropertyLimits>
{
public:
  using NumericProperty::NumericProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "IntegerProperty";
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
