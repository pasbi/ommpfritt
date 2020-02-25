#pragma once

#include "properties/optionsproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{

class OptionsEdit;

class OptionsPropertyWidget : public PropertyWidget<OptionsProperty>
{
public:
  explicit OptionsPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  OptionsEdit* combobox() const { return m_options_edit; }

protected:
  void update_edit() override;
  QString type() const override;

private:
  OptionsEdit* m_options_edit;
};

}  // namespace omm
