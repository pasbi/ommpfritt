#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/transformationproperty.h"

namespace omm
{

class AbstractTransformationEdit;

class TransformationPropertyWidget : public PropertyWidget<TransformationProperty>
{
public:
  explicit TransformationPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);
  std::string type() const override;

protected:
  void on_property_value_changed() override;

  AbstractTransformationEdit* m_transformation_edit;
};

}  // namespace omm
