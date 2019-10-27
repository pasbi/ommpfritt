#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class BoolProperty : public TypedProperty<bool>
{
public:
  using TypedProperty::TypedProperty;
  QString type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "BoolProperty");
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
