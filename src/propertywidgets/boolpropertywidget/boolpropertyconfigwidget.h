#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/boolproperty.h"

namespace omm
{

class BoolPropertyConfigWidget : public PropertyConfigWidget<BoolProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
