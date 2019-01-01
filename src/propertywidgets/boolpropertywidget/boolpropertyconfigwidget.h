#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/boolproperty.h"

namespace omm
{

class BoolPropertyConfigWidget : public PropertyConfigWidget<BoolProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
