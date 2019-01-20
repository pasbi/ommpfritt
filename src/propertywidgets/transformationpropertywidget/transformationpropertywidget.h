#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/transformationproperty.h"

namespace omm
{

class SpinBoxTransformationEdit;

class TransformationPropertyWidget : public PropertyWidget<TransformationProperty>
{
public:
  explicit TransformationPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  std::string type() const override;

protected:
  void update_edit() override;

  SpinBoxTransformationEdit* m_transformation_edit;
};

}  // namespace omm
