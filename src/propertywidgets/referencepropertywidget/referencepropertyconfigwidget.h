#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/referenceproperty.h"

namespace omm
{

class ReferencePropertyConfigWidget : public PropertyConfigWidget<ReferenceProperty>
{
  Q_OBJECT
public:
  ReferencePropertyConfigWidget(QWidget* parent, Property& property);
  std::string type() const override;
};

}  // namespace omm
