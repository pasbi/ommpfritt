#include "properties/property.h"
#include <algorithm>
#include <cassert>
#include "objects/object.h"

#include "properties/optionsproperty.h"
#include <Qt>

namespace omm
{

const std::string Property::USER_PROPERTY_CATEGROY_NAME = QT_TRANSLATE_NOOP( "Property",
                                                                             "user properties" );

void Property::notify_observers()
{
  if (!m_notifications_are_blocked) {
    Observed<AbstractPropertyObserver>::for_each([this](auto* observer) {
      observer->on_property_value_changed(*this);
    });
  }
}

std::string Property::label() const { return m_label; }
std::string Property::widget_type() const { return type() + "Widget"; }
std::string Property::category() const { return m_category; }
bool Property::is_user_property() const { return m_category == USER_PROPERTY_CATEGROY_NAME; }
OptionsProperty* Property::enabled_buddy() const { return m_enabled_buddy.property; }
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
{;
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

bool Property::is_compatible(const Property& other) const
{
  return other.category() == category() && other.type() == type();
}

Property::NotificationBlocker::NotificationBlocker(Property &p) : m_p(p)
{
  m_p.m_notifications_are_blocked = true;
}

Property::NotificationBlocker::~NotificationBlocker()
{
  m_p.m_notifications_are_blocked = false;
}

}  // namespace omm
