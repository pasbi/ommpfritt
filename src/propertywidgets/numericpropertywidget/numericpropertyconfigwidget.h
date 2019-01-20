#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

class IntegerPropertyConfigWidget : public PropertyConfigWidget<IntegerProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

class FloatPropertyConfigWidget : public PropertyConfigWidget<FloatProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
