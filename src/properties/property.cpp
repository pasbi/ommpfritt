#include "properties/property.h"
#include <algorithm>
#include <assert.h>
#include "objects/object.h"

#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "properties/transformationproperty.h"

namespace omm
{

Property::Property()
{
}

Property::~Property()
{

}

std::string Property::label() const
{
  return m_label;
}

std::string Property::category() const
{
  return m_category;
}

Property& Property::set_label(const std::string& label)
{
  m_label = label;
  return *this;
}

Property& Property::set_category(const std::string& category)
{
  m_category = category;
  return *this;
}

void Property::register_properties()
{
#define REGISTER_PROPERTY(TYPE) Property::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(TransformationProperty);
  // REGISTER_PROPERTY(ReferenceProperty);

#undef REGISTER_PROPERTY
}

}  // namespace omm
