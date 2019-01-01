#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/colorproperty.h"

namespace omm
{

class ColorPropertyConfigWidget : public PropertyConfigWidget<ColorProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
