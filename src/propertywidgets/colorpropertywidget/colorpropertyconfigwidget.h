#pragma once

#include "properties/colorproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class ColorPropertyConfigWidget : public PropertyConfigWidget<ColorProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const PropertyConfiguration&) override
  {
  }
  void update(PropertyConfiguration&) const override
  {
  }
};

}  // namespace omm
