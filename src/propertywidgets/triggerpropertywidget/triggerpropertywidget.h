#pragma once

#include "properties/triggerproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{
class TriggerPropertyWidget : public PropertyWidget<TriggerProperty>
{
public:
  explicit TriggerPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  void trigger();

protected:
  void update_edit() override;
};

}  // namespace omm
