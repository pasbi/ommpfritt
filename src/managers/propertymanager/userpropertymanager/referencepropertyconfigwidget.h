#pragma once

#include "managers/propertymanager/userpropertymanager/propertyconfigwidget.h"
#include "properties/referenceproperty.h"

namespace omm
{

class ReferencePropertyConfigWidget : public PropertyConfigWidget<ReferenceProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
