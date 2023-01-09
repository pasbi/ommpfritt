#pragma once

#include "properties/facelistproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

class QListWidget;

namespace omm
{

class FaceListPropertyConfigWidget : public PropertyConfigWidget<FaceListProperty>
{
  Q_OBJECT
};

}  // namespace omm
