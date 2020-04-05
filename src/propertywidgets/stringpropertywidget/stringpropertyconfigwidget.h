#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class StringPropertyConfigWidget : public PropertyConfigWidget<StringProperty>
{
public:
  StringPropertyConfigWidget();
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;

private:
  QComboBox* m_mode_selector;
};

}  // namespace omm
