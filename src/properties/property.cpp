#include "properties/property.h"
#include "objects/object.h"
#include <algorithm>
#include <cassert>

#include "animation/track.h"
#include "properties/optionproperty.h"
#include <Qt>

namespace omm
{
std::map<QString, const Property::PropertyDetail*> Property::m_details;

// NOLINTNEXTLINE(readability-redundant-member-init)
Property::Property(const Property& other) : QObject(), configuration(other.configuration)
{
}

QString Property::widget_type() const
{
  return type() + "Widget";
}
bool Property::is_user_property() const
{
  return category() == USER_PROPERTY_CATEGROY_NAME;
}
void Property::revise()
{
}

void Property::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  if (is_user_property()) {
    serializer.set_value(label(), make_pointer(root, LABEL_POINTER));
    serializer.set_value(category(), make_pointer(root, CATEGORY_POINTER));
  }
  serializer.set_value(is_animatable(), make_pointer(root, ANIMATABLE_POINTER));
  serializer.set_value(m_track != nullptr, make_pointer(root, IS_ANIMATED_POINTER));
  if (m_track != nullptr) {
    m_track->serialize(serializer, make_pointer(root, TRACK_POINTER));
  }
}

void Property ::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  configuration.deserialize_field<QString>(LABEL_POINTER, deserializer, root);
  configuration.deserialize_field<QString>(CATEGORY_POINTER, deserializer, root);
  configuration.deserialize_field<bool>(ANIMATABLE_POINTER, deserializer, root);

  if (deserializer.get_bool(make_pointer(root, IS_ANIMATED_POINTER))) {
    m_track = std::make_unique<Track>(*this);
    m_track->deserialize(deserializer, make_pointer(root, TRACK_POINTER));
  }
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

QString Property::label() const
{
  return configuration.get<QString>(LABEL_POINTER);
}

Property& Property::set_label(const QString& label)
{
  configuration.set(LABEL_POINTER, label);
  return *this;
}

QString Property::category() const
{
  return configuration.get<QString>(CATEGORY_POINTER);
}

Property& Property::set_category(const QString& category)
{
  configuration.set(CATEGORY_POINTER, category);
  return *this;
}

bool Property::is_animatable() const
{
  return configuration.get(ANIMATABLE_POINTER, true);
}

Property& Property::set_animatable(bool animatable)
{
  configuration.set(ANIMATABLE_POINTER, animatable);
  return *this;
}

Track* Property::track() const
{
  return m_track.get();
}

void Property::set_track(std::unique_ptr<Track> track)
{
  assert(&track->property() == this);
  m_track = std::move(track);
}

std::unique_ptr<Track> Property::extract_track()
{
  return std::move(m_track);
}

std::size_t Property::n_channels() const
{
  return omm::n_channels(variant_value());
}

double Property::channel_value(std::size_t channel) const
{
  return omm::get_channel_value(variant_value(), channel);
}

void Property::set_channel_value(std::size_t channel, double value)
{
  auto variant = variant_value();
  omm::set_channel_value(variant, channel, value);
  set(variant);
}

QString Property::channel_name(std::size_t channel) const
{
  return m_details.at(type())->channel_name(*this, channel);
}

QString Property::data_type() const
{
  const QString type = this->type();
  static const QString suffix = "Property";
  assert(type.endsWith(suffix));
  return type.mid(0, type.size() - suffix.size());
}

void Property::set_enabledness(bool enabled)
{
  if (m_is_enabled != enabled) {
    m_is_enabled = enabled;
    Q_EMIT enabledness_changed(enabled);
  }
}

}  // namespace omm
