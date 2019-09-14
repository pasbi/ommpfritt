#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{

class IntegerProperty : public NumericProperty<int>
{
public:
  using NumericProperty::NumericProperty;
  std::string type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "IntegerProperty");
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
