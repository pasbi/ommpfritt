#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{

struct FloatPropertyLimits
{
  static constexpr auto upper =  std::numeric_limits<double>::infinity();
  static constexpr auto lower = -std::numeric_limits<double>::infinity();
  static constexpr auto step = 1.0;
};

class FloatProperty : public NumericProperty<double, FloatPropertyLimits>
{
public:
  using NumericProperty::NumericProperty;
  std::string type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "FloatProperty");
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
