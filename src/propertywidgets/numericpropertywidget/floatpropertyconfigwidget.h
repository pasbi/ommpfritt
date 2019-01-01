#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/floatproperty.h"

namespace omm
{

class FloatPropertyConfigWidget : public PropertyConfigWidget<FloatProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
