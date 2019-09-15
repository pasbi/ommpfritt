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
  , configuration(other.configuration)
{
}

std::string Property::widget_type() const { return type() + "Widget"; }
bool Property::is_user_property() const { return category() == USER_PROPERTY_CATEGROY_NAME; }
void Property::revise() {}

void Property::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  serializer.set_value(label(), make_pointer(root, LABEL_POINTER));
  serializer.set_value(category(), make_pointer(root, CATEGORY_POINTER));
  serializer.set_value(is_animatable(), make_pointer(root, ANIMATABLE_POINTER));
}

void Property
::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  // if m_label and m_category are already set, prefer these values since they are translated.
  // if m_label and m_category are not set, use the loaded ones (useful for user properties)
  if (configuration.find(LABEL_POINTER) == configuration.end()) {
    configuration[LABEL_POINTER] = deserializer.get_string(make_pointer(root, LABEL_POINTER));
  }
  if (configuration.find(CATEGORY_POINTER) == configuration.end()) {
    configuration[CATEGORY_POINTER] = deserializer.get_string(make_pointer(root, CATEGORY_POINTER));
  }
  if (configuration.find(ANIMATABLE_POINTER) == configuration.end()) {
    configuration[ANIMATABLE_POINTER] = deserializer.get_bool(make_pointer(root, ANIMATABLE_POINTER));
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

std::string Property::label() const
{
  return std::get<std::string>(configuration.at(LABEL_POINTER));
}

Property& Property::set_label(const std::string& label)
{
  configuration[LABEL_POINTER] = label;
  return *this;
}

std::string Property::category() const
{
  return std::get<std::string>(configuration.at(CATEGORY_POINTER));
}

Property& Property::set_category(const std::string& category)
{
  configuration[CATEGORY_POINTER] = category;
  return *this;
}

bool Property::is_animatable() const
{
  return configuration.get(ANIMATABLE_POINTER, true);
}

Property &Property::set_animatable(bool animatable)
{
  configuration[ANIMATABLE_POINTER] = animatable;
  return *this;
}

}  // namespace omm
