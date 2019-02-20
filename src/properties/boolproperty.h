#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class BoolProperty : public TypedProperty<bool>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TR_NOOP("BoolProperty");
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
