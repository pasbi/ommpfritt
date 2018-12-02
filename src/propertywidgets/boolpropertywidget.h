#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/boolproperty.h"

class QLineEdit;

namespace omm
{

class BoolPropertyWidget : public PropertyWidget<BoolProperty>
{
public:
  explicit BoolPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);

protected:
  void on_property_value_changed() override;
  std::string type() const override;

private:
  class CheckBox;
  CheckBox* m_checkbox;
};

}  // namespace omm
