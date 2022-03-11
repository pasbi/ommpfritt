#include "properties/property.h"
#include "objects/object.h"
#include <algorithm>
#include <cassert>

#include "animation/track.h"
#include "properties/optionproperty.h"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"
#include <Qt>

namespace omm
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::map<QString, const Property::PropertyDetail*> Property::m_details;

// NOLINTNEXTLINE(readability-redundant-member-init)
Property::Property() = default;

// NOLINTNEXTLINE(readability-redundant-member-init,-warnings-as-errors)
Property::Property(const Property& other) : QObject(), configuration(other.configuration)
{
}

Property::~Property() = default;

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

void Property::serialize(serialization::SerializerWorker& worker) const
{
  if (is_user_property()) {
    worker.sub(LABEL_POINTER)->set_value(label());
    worker.sub(CATEGORY_POINTER)->set_value(category());
  }
  worker.sub(ANIMATABLE_POINTER)->set_value(is_animatable());
  worker.sub(IS_ANIMATED_POINTER)->set_value(m_track != nullptr);
  if (m_track != nullptr) {
    m_track->serialize(*worker.sub(TRACK_POINTER));
  }
}

void Property::deserialize(serialization::DeserializerWorker& worker)
{
  configuration.deserialize_field<QString>(LABEL_POINTER, worker);
  configuration.deserialize_field<QString>(CATEGORY_POINTER, worker);

  configuration.set(ANIMATABLE_POINTER, worker.sub(ANIMATABLE_POINTER)->get_bool());
  if (worker.sub(IS_ANIMATED_POINTER)->get_bool()) {
    m_track = std::make_unique<Track>(*this);
    m_track->deserialize(*worker.sub(TRACK_POINTER));
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
  return configuration.get<QString>(CATEGORY_POINTER, "");
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

Type Property::data_type() const
{
  const QString type = this->type();
  static const QString suffix = "Property";
  assert(type.endsWith(suffix));
  const auto data_type = type.mid(0, type.size() - suffix.size());
  return get_variant_type(data_type.toStdString());
}

void Property::set_enabledness(bool enabled)
{
  if (m_is_enabled != enabled) {
    m_is_enabled = enabled;
    Q_EMIT enabledness_changed(enabled);
  }
}

}  // namespace omm
