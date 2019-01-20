#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/vectorproperty.h"

namespace omm
{

class IntegerVectorPropertyConfigWidget : public PropertyConfigWidget<IntegerVectorProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

class FloatVectorPropertyConfigWidget : public PropertyConfigWidget<FloatVectorProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
