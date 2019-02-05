#include "properties/property.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "properties/optionsproperty.h"

namespace omm
{

void register_properties()
{
#define REGISTER_PROPERTY(TYPE) Property::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTY(BoolProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(ReferenceProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(OptionsProperty);


#undef REGISTER_PROPERTY
}

}  // namespace omm


