#include "properties/property.h"
#include <algorithm>
#include <cassert>
#include "objects/object.h"

#include "properties/optionproperty.h"
#include <Qt>
#include "animation/track.h"

namespace omm
{

std::map<QString, const Property::PropertyDetail*> Property::m_details;

Property::Property(const Property &other)
  : QObject()
  , configuration(other.configuration)
{
}

QString Property::widget_type() const { return type() + "Widget"; }
bool Property::is_user_property() const { return category() == USER_PROPERTY_CATEGROY_NAME; }
void Property::revise() {}

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

void Property
::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  if (configuration.find(LABEL_POINTER) == configuration.end()) {
    configuration[LABEL_POINTER] = deserializer.get_string(make_pointer(root, LABEL_POINTER));
  }
  if (configuration.find(CATEGORY_POINTER) == configuration.end()) {
    configuration[CATEGORY_POINTER] = deserializer.get_string(make_pointer(root, CATEGORY_POINTER));
  }
  if (configuration.find(ANIMATABLE_POINTER) == configuration.end()) {
    configuration[ANIMATABLE_POINTER] = deserializer.get_bool(make_pointer(root, ANIMATABLE_POINTER));
  }
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
  return std::get<QString>(configuration.at(LABEL_POINTER));
}

Property& Property::set_label(const QString& label)
{
  configuration[LABEL_POINTER] = label;
  return *this;
}

QString Property::category() const
{
  return std::get<QString>(configuration.at(CATEGORY_POINTER));
}

Property& Property::set_category(const QString& category)
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

Track *Property::track() const
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

void Property::Filter::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  deserializer.get(kind, make_pointer(root, "kind"));
  deserializer.get(flag, make_pointer(root, "flag"));
}

bool Property::Filter::accepts(Kind kind, Flag flag) const
{
  return this->flag.evaluate(flag) && this->kind.evaluate(kind);
}

void Property::Filter::serialize(AbstractSerializer &serializer, const Pointer &root) const
{
  serializer.set_value(kind, make_pointer(root, "kind"));
  serializer.set_value(flag, make_pointer(root, "flag"));
}

bool Property::Filter::accepts(const AbstractPropertyOwner& apo) const
{
  return accepts(apo.kind, apo.flags());
}

bool Property::Filter::operator==(const Property::Filter& other) const
{
  return kind == other.kind && flag == other.flag;
}

bool Property::Filter::operator<(const Property::Filter& other) const
{
  if (kind == other.kind) {
    return flag < other.flag;
  } else {
    return kind == other.kind;
  }
}

Property::Filter Property::Filter::accept_anything()
{
  return Filter(Disjunction<Kind>(Kind::All, Kind::None), { {} });
}

std::ostream& operator<<(std::ostream& ostream, const Property::Filter& filter)
{
  ostream << "Filter(Flag(" << filter.flag << "), Kind(" << filter.kind << "))";
  return ostream;
}

}  // namespace omm
