#pragma once

#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{

class ColorProperty;

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
