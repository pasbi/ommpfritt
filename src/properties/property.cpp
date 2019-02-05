#include "properties/property.h"
#include <algorithm>
#include <cassert>
#include "objects/object.h"

#include "properties/optionsproperty.h"

namespace omm
{

const std::string Property::USER_PROPERTY_CATEGROY_NAME = "user properties"; // TODO translate

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

Property& Property::set_pre_submit(const std::function<void(Property&)>& hook)
{
  pre_submit = hook;
  wrap_with_macro = true;
  return *this;
}

Property& Property::set_post_submit(const std::function<void(Property&)>& hook)
{
  post_submit = hook;
  wrap_with_macro = true;
  return *this;
}

bool TriggerPropertyDummyValueType::operator==(const TriggerPropertyDummyValueType&) const
{
  return true;
}

bool TriggerPropertyDummyValueType::operator!=(const TriggerPropertyDummyValueType& other) const
{
  return !(this->operator==(other));
}

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValueType& v)
{
  ostream << "[TriggerPropertyDummyValueType]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Property::variant_type& v)
{
  print_variant_value(ostream, v);
  return ostream;
}

OptionsProperty* Property::enabled_buddy() const
{
  return m_enabled_buddy.property;
}

bool Property::is_enabled() const
{
  if (m_enabled_buddy.property == nullptr) { return true; }
  else {
    return ::contains(m_enabled_buddy.target_values, m_enabled_buddy.property->value());
  }
}

Property&
Property::set_enabled_buddy(OptionsProperty& property, const std::set<std::size_t>& values)
{
  m_enabled_buddy.property = &property;
  m_enabled_buddy.target_values = values;
  return *this;
}


}  // namespace omm
