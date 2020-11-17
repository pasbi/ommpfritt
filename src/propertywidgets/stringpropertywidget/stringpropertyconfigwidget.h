#pragma once

#include "properties/stringproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class StringPropertyConfigWidget : public PropertyConfigWidget<StringProperty>
{
public:
  StringPropertyConfigWidget();
  void init(const Property::Configuration& configuration) override;
  void update(Property::Configuration& configuration) const override;

private:
  QComboBox* m_mode_selector;
};

}  // namespace omm
