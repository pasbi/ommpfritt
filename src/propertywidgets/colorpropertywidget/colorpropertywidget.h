#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/colorproperty.h"

namespace omm
{

class ColorEdit;

class ColorPropertyWidget : public PropertyWidget<ColorProperty>
{
public:
  explicit ColorPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  std::string type() const override;
  static constexpr auto TYPE = "ColorPropertyWidget";

private:
  ColorEdit* m_color_edit;
};

}  // namespace omm
