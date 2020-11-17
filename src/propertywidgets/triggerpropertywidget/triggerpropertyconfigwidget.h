#pragma once

#include "properties/triggerproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class TriggerPropertyConfigWidget : public PropertyConfigWidget<TriggerProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const Property::Configuration&) override
  {
  }
  void update(Property::Configuration&) const override
  {
  }
};

}  // namespace omm
