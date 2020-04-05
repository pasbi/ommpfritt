#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/colorproperty.h"

namespace omm
{

class ColorPropertyConfigWidget : public PropertyConfigWidget<ColorProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
