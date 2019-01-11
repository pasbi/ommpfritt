#include "propertywidgets/boolpropertywidget/boolpropertyconfigwidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertyconfigwidget.h"
#include "propertywidgets/numericpropertywidget/floatpropertyconfigwidget.h"
#include "propertywidgets/numericpropertywidget/integerpropertyconfigwidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include "propertywidgets/transformationpropertywidget/transformationpropertyconfigwidget.h"
#include "propertywidgets/optionspropertywidget/optionspropertyconfigwidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertyconfigwidget.h"

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
  REGISTER_PROPERTY(OptionsPropertyConfigWidget);
  REGISTER_PROPERTY(TriggerPropertyConfigWidget);

#undef REGISTER_PROPERTY
}

}  // namespace omm