#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/triggerproperty.h"

namespace omm
{

class TriggerPropertyConfigWidget : public PropertyConfigWidget<TriggerProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
