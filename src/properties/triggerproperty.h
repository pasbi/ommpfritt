#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class TriggerProperty : public TypedProperty<TriggerPropertyDummyValueType>
{
public:
  using TypedProperty::TypedProperty;
  QString type() const override { return TYPE; }
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "TriggerProperty");
  std::unique_ptr<Property> clone() const override;
  void trigger();
  static const PropertyDetail detail;
};

}  // namespace omm
