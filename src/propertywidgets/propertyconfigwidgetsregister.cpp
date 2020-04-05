#include "propertywidgets/boolpropertywidget/boolpropertyconfigwidget.h"
#include "propertywidgets/boolpropertywidget/boolpropertywidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertyconfigwidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertywidget.h"
#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"
#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertywidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"
#include "propertywidgets/optionspropertywidget/optionspropertyconfigwidget.h"
#include "propertywidgets/optionspropertywidget/optionspropertywidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertyconfigwidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertywidget.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertywidget.h"
#include "propertywidgets/splinepropertywidget/splinepropertyconfigwidget.h"
#include "propertywidgets/splinepropertywidget/splinepropertywidget.h"

namespace omm
{

void register_propertyconfigwidgets()
{
#define REGISTER_PROPERTY(TYPE) \
  Property::register_type<TYPE>(#TYPE); \
  AbstractPropertyWidget::register_type<TYPE##Widget>(#TYPE"Widget"); \
  AbstractPropertyConfigWidget::register_type<TYPE##ConfigWidget>(#TYPE"ConfigWidget");

  REGISTER_PROPERTY(BoolProperty);
  REGISTER_PROPERTY(ColorProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(ReferenceProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(OptionsProperty);
  REGISTER_PROPERTY(TriggerProperty);
  REGISTER_PROPERTY(IntegerVectorProperty);
  REGISTER_PROPERTY(FloatVectorProperty);
  REGISTER_PROPERTY(SplineProperty)

#undef REGISTER_PROPERTY
}

}  // namespace omm
