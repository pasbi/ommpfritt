#include "animation/abstracttrack.h"
#include "logging.h"

namespace omm
{

void AbstractTrack:: serialize(AbstractSerializer& serializer, const Pointer& pointer) const
{
  serializer.set_value(m_property_key, make_pointer(pointer, PROPERTY_KEY_KEY));
  serializer.set_value(m_owner, make_pointer(pointer, OWNER_KEY));

  const auto keys = this->keys();
  const auto keyvalues_pointer = make_pointer(pointer, KEY_VALUES_KEY);
  serializer.start_array(keys.size(), keyvalues_pointer);

  for (std::size_t i = 0; i < count(); ++i) {
    serialize_keyvalue(serializer, i, keyvalues_pointer);
  }
  serializer.end_array();
}

void AbstractTrack:: deserialize(AbstractDeserializer& deserializer, const Pointer& pointer)
{
  m_property_key = deserializer.get_string(make_pointer(pointer, PROPERTY_KEY_KEY));
  m_owner_id = deserializer.get_size_t(make_pointer(pointer, OWNER_KEY));
  deserializer.register_reference_polisher(*this);

  const auto keyvalues_pointer = make_pointer(pointer, KEY_VALUES_KEY);
  const std::size_t n = deserializer.array_size(keyvalues_pointer);
  for (std::size_t i = 0; i < n; ++i) {
    deserialize_keyvalue(deserializer, i, keyvalues_pointer);
  }
}

AbstractPropertyOwner *AbstractTrack::owner() const
{
  return m_owner;
}

const std::string AbstractTrack::property_key() const
{
  return m_property_key;
}

void AbstractTrack::set_owner(AbstractPropertyOwner &owner, const std::string &property_key)
{
  assert(m_owner == nullptr && m_property_key.empty());
  m_owner = &owner;
  m_property_key = property_key;
}

std::string AbstractTrack::map_property_to_track_type(const std::string& property_type)
{
  static const std::string property_suffix = "Property";
  static const std::string track_suffix = "Track";
  assert(property_type.size() > property_suffix.size());
  return property_type.substr(0, property_type.size() - property_suffix.size()) + track_suffix;
}

}  // namespace omm
