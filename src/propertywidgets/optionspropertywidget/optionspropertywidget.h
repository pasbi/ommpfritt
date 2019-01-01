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

protected:
  void update_edit() override;
  std::string type() const override;
  static constexpr auto TYPE = "OptionsPropertyWidget";

private:
  OptionsEdit* m_options_edit;
};

}  // namespace omm
