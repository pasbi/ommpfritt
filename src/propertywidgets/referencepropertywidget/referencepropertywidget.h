#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"
#include "properties/referenceproperty.h"

class QLineEdit;

namespace omm
{

class ReferenceLineEdit;

class ReferencePropertyWidget : public PropertyWidget<ReferenceProperty>
{
public:
  explicit ReferencePropertyWidget(Scene& scene, const Property::SetOfProperties& properties);
  ~ReferencePropertyWidget();

protected:
  void update_edit() override;
  std::string type() const override;

private:
  ReferenceLineEdit* m_line_edit;
};

}  // namespace omm
