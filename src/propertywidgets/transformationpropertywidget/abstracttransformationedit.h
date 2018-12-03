#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"
#include "propertywidgets/multivalueedit.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class AbstractTransformationEdit
  : public QWidget
  , public MultiValueEdit<ObjectTransformation>
{
  Q_OBJECT

Q_SIGNALS:
  void value_changed(const ObjectTransformation& object_transformation);
};

}  // namespace omm
