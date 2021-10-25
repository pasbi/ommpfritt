#pragma once

#include "properties/boolproperty.h"
#include "propertywidgets/propertywidget.h"

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

private:
  CheckBox* m_checkbox;
};

}  // namespace omm
