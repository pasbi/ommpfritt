#include "managers/propertymanager/userpropertymanager/boolpropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/colorpropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/floatpropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/integerpropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/referencepropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/stringpropertyconfigwidget.h"
#include "managers/propertymanager/userpropertymanager/transformationpropertyconfigwidget.h"

namespace omm
{

void register_propertyconfigwidgets()
{
#define REGISTER_PROPERTY(TYPE) \
  AbstractPropertyConfigWidget::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTY(BoolPropertyConfigWidget);
  REGISTER_PROPERTY(ColorPropertyConfigWidget);
  REGISTER_PROPERTY(FloatPropertyConfigWidget);
  REGISTER_PROPERTY(IntegerPropertyConfigWidget);
  REGISTER_PROPERTY(ReferencePropertyConfigWidget);
  REGISTER_PROPERTY(StringPropertyConfigWidget);
  REGISTER_PROPERTY(TransformationPropertyConfigWidget);

#undef REGISTER_PROPERTY
}

}  // namespace omm