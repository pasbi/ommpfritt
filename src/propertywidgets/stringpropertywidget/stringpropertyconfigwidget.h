#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class StringPropertyConfigWidget : public PropertyConfigWidget
{
public:
  StringPropertyConfigWidget();
  static constexpr auto TYPE = "StringPropertyConfigWidget";
  std::string type() const override { return TYPE; }
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;

private:
  QComboBox* m_mode_selector;
};

}  // namespace omm
