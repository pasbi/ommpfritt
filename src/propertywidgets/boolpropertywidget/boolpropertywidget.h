#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/boolproperty.h"

class QLineEdit;

namespace omm
{

class CheckBox;

class BoolPropertyWidget : public PropertyWidget<BoolProperty>
{
public:
  explicit BoolPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  std::string type() const override;
  static constexpr auto TYPE = "BoolPropertyWidget";

private:
  CheckBox* m_checkbox;
};

}  // namespace omm
