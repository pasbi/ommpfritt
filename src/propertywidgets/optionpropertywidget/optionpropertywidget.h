#pragma once

#include "properties/optionproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{
class OptionsEdit;

class OptionPropertyWidget : public PropertyWidget<OptionProperty>
{
public:
  explicit OptionPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  [[nodiscard]] OptionsEdit* combobox() const
  {
    return m_options_edit;
  }

protected:
  void update_edit() override;

private:
  OptionsEdit* m_options_edit;
};

}  // namespace omm
