#include "properties/property.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "properties/optionsproperty.h"
#include "properties/vectorproperty.h"
#include "properties/colorproperty.h"
#include "properties/triggerproperty.h"
#include "properties/splineproperty.h"

namespace omm
{

void register_properties()
{
#define REGISTER_PROPERTY(TYPE) \
  Property::register_type<TYPE>(#TYPE); \
  Property::m_details[#TYPE] = &TYPE::detail;

  REGISTER_PROPERTY(BoolProperty);
  REGISTER_PROPERTY(ColorProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(ReferenceProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(OptionsProperty);
  REGISTER_PROPERTY(FloatVectorProperty);
  REGISTER_PROPERTY(IntegerVectorProperty);
  REGISTER_PROPERTY(TriggerProperty);
  REGISTER_PROPERTY(SplineProperty);

#undef REGISTER_PROPERTY
}

}  // namespace omm


