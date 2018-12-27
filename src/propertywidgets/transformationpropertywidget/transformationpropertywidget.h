#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/transformationproperty.h"

namespace omm
{

class AbstractTransformationEdit;

class TransformationPropertyWidget : public PropertyWidget<TransformationProperty>
{
public:
  explicit TransformationPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  std::string type() const override;
  static constexpr auto TYPE = "TransformationPropertyWidget";

protected:
  void update_edit() override;

  AbstractTransformationEdit* m_transformation_edit;
};

}  // namespace omm
