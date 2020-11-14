#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{
class TriggerProperty : public TypedProperty<TriggerPropertyDummyValueType>
{
public:
  using TypedProperty::TypedProperty;
  void trigger();
  static const PropertyDetail detail;
};

}  // namespace omm
