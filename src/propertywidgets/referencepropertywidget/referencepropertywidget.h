#pragma once

#include "properties/referenceproperty.h"
#include "propertywidgets/propertywidget.h"
#include <string>

class QLineEdit;

namespace omm
{
class ReferenceLineEdit;

class ReferencePropertyWidget : public PropertyWidget<ReferenceProperty>
{
public:
  explicit ReferencePropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;

private:
  ReferenceLineEdit* m_line_edit;
};

}  // namespace omm
