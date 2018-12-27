#pragma once

#include "managers/propertymanager/userpropertymanager/propertyconfigwidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class StringPropertyConfigWidget : public PropertyConfigWidget<StringProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
