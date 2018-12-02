#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/colorproperty.h"

namespace omm
{

class ColorEdit;

class ColorPropertyWidget : public PropertyWidget<ColorProperty>
{
public:
  explicit ColorPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);

protected:
  void on_property_value_changed() override;
  std::string type() const override;

private:
  ColorEdit* m_color_edit;
};

}  // namespace omm
