#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{

class FloatProperty : public NumericProperty<double>
{
public:
  using NumericProperty::NumericProperty;
  QString type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "FloatProperty");
  static const PropertyDetail detail;
};

}  // namespace omm
