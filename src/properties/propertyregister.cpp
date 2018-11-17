#include "properties/property.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"
#include "properties/stringproperty.h"
#include "properties/transformationproperty.h"
#include "properties/referenceproperty.h"

namespace omm
{

void register_properties()
{
#define REGISTER_PROPERTY(TYPE) Property::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(TransformationProperty);
  REGISTER_PROPERTY(ReferenceProperty);

#undef REGISTER_PROPERTY
}

}  // namespace omm


