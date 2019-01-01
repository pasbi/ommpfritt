#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/referenceproperty.h"

namespace omm
{

class ReferencePropertyConfigWidget : public PropertyConfigWidget<ReferenceProperty>
{
public:
  ReferencePropertyConfigWidget(QWidget* parent, Property& property);
  std::string type() const override;
};

}  // namespace omm
