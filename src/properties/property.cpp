#include "properties/property.h"
#include <algorithm>
#include <cassert>
#include "objects/object.h"

#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "properties/transformationproperty.h"

namespace omm
{

const std::string Property::USER_PROPERTY_CATEGROY_NAME = "user properties"; // TODO translate

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

std::string Property::widget_type() const
{
  return type() + "Widget";
}

void Property::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  serializer.set_value(m_label, make_pointer(root, "label"));
  serializer.set_value(m_category, make_pointer(root, "category"));
}

void Property
::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);
  m_label = deserializer.get_string(make_pointer(root, "label"));
  m_category = deserializer.get_string(make_pointer( root, "category" ));
}

bool Property::is_compatible(const Property& other) const
{
  return type() == other.type();
}

bool Property::is_user_property() const
{
  return m_category == USER_PROPERTY_CATEGROY_NAME;
}

}  // namespace omm
