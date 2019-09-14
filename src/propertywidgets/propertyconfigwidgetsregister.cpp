#include "propertywidgets/boolpropertywidget/boolpropertyconfigwidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertyconfigwidget.h"
#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include "propertywidgets/optionspropertywidget/optionspropertyconfigwidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertyconfigwidget.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"

namespace omm
{

void register_propertyconfigwidgets()
{
#define REGISTER_PROPERTY(TYPE) \
  PropertyConfigWidget::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTY(BoolPropertyConfigWidget);
  REGISTER_PROPERTY(ColorPropertyConfigWidget);
  REGISTER_PROPERTY(FloatPropertyConfigWidget);
  REGISTER_PROPERTY(IntegerPropertyConfigWidget);
  REGISTER_PROPERTY(ReferencePropertyConfigWidget);
  REGISTER_PROPERTY(StringPropertyConfigWidget);
  REGISTER_PROPERTY(OptionsPropertyConfigWidget);
  REGISTER_PROPERTY(TriggerPropertyConfigWidget);
  REGISTER_PROPERTY(IntegerVectorPropertyConfigWidget);
  REGISTER_PROPERTY(FloatVectorPropertyConfigWidget);

#undef REGISTER_PROPERTY
}

}  // namespace omm
