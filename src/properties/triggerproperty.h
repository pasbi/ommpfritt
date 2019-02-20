#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class TriggerProperty : public TypedProperty<TriggerPropertyDummyValueType>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TR_NOOP("TriggerProperty");
  std::unique_ptr<Property> clone() const override;
  void trigger();
};

}  // namespace omm
