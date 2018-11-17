#include "propertywidgets/propertywidget.h"
#include "propertywidgets/numericpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/transformationpropertywidget.h"
#include "propertywidgets/referencepropertywidget.h"

namespace omm
{

void register_propertywidgets()
{
#define REGISTER_PROPERTYWIDGET(TYPE) AbstractPropertyWidget::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTYWIDGET(IntegerPropertyWidget);
  REGISTER_PROPERTYWIDGET(FloatPropertyWidget);
  REGISTER_PROPERTYWIDGET(StringPropertyWidget);
  REGISTER_PROPERTYWIDGET(TransformationPropertyWidget);
  REGISTER_PROPERTYWIDGET(ReferencePropertyWidget);

#undef REGISTER_PROPERTYWIDGET
}

}  // namespace omm
