#include "properties/property.h"
#include <algorithm>
#include <cassert>
#include "objects/object.h"

#include "properties/optionsproperty.h"
#include <Qt>

namespace omm
{

Property::Property(const Property &other)
  : QObject()
  , m_label(other.m_label)
  , m_category(other.m_category)
{
}

std::string Property::label() const { return m_label; }
std::string Property::widget_type() const { return type() + "Widget"; }
std::string Property::category() const { return m_category; }
bool Property::is_user_property() const { return m_category == USER_PROPERTY_CATEGROY_NAME; }
void Property::revise() {}

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

  // if m_label and m_category are already set, prefer these values since they are translated.
  // if m_label and m_category are not set, use the loaded ones (useful for user properties)
  if (m_label.empty()) {
    m_label = deserializer.get_string(make_pointer(root, "label"));
  }
  if (m_category.empty()) {
    m_category = deserializer.get_string(make_pointer( root, "category"));
  }
}

bool TriggerPropertyDummyValueType::operator==(const TriggerPropertyDummyValueType&) const
{
  return true;
}

bool TriggerPropertyDummyValueType::operator!=(const TriggerPropertyDummyValueType& other) const
{
  return !(this->operator==(other));
}

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValueType&)
{
  ostream << "[TriggerPropertyDummyValueType]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Property::variant_type& v)
{
  print_variant_value(ostream, v);
  return ostream;
}

bool Property::is_visible() const
{
  return m_is_visible;
}

void Property::set_visible(bool visible)
{
  if (m_is_visible != visible) {
    m_is_visible = visible;
    Q_EMIT visibility_changed(visible);
  }
}

bool Property::is_compatible(const Property& other) const
{
  return other.category() == category() && other.type() == type();
}

}  // namespace omm
