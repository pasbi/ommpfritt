#pragma once

#include "managers/propertymanager/userpropertymanager/propertyconfigwidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class StringPropertyConfigWidget : public PropertyConfigWidget<StringProperty>
{
public:
  StringPropertyConfigWidget(QWidget* parent, Property& property);
  std::string type() const override;
};

}  // namespace omm
