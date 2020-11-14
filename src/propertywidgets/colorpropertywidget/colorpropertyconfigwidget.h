#pragma once

#include "properties/colorproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class ColorPropertyConfigWidget : public PropertyConfigWidget<ColorProperty>
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
