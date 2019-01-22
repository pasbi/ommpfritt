#include "propertywidgets/transformationpropertywidget/transformationpropertywidget.h"
#include "propertywidgets/transformationpropertywidget/spinboxtransformationedit.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

TransformationPropertyWidget
::TransformationPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  const auto on_value_changed = [this](const auto& value) { this->set_properties_value(value); };
  auto transformation_edit = std::make_unique<SpinBoxTransformationEdit>(on_value_changed);
  m_transformation_edit = transformation_edit.get();
  set_default_layout(std::move(transformation_edit));
  update_edit();
}

void TransformationPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_transformation_edit);
  m_transformation_edit->set_values(get_properties_values());
}

std::string TransformationPropertyWidget::type() const
{
  return TYPE;
}


} // namespace omm
