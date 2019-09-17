#include "animation/track.h"
#include "logging.h"
#include "aspects/propertyowner.h"
#include "common.h"

namespace omm
{

void Track::serialize(AbstractSerializer& serializer, const Pointer& pointer) const
{
  serializer.set_value(m_property_key, make_pointer(pointer, PROPERTY_KEY_KEY));
  serializer.set_value(m_owner, make_pointer(pointer, OWNER_KEY));
  serializer.set_value(type(), make_pointer(pointer, TYPE_KEY));

  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const auto key_frames = this->key_frames();
  serializer.start_array(key_frames.size(), knots_pointer);
  for (std::size_t i = 0; i < key_frames.size(); ++i) {
    const int frame = key_frames.at(i);
    const Knot& knot = m_knots.at(frame);
    const auto knot_pointer = make_pointer(knots_pointer, i);
    serializer.set_value(frame, make_pointer(knot_pointer, FRAME_KEY));
    serializer.set_value(knot.value, make_pointer(knot_pointer, VALUE_KEY));
    serializer.set_value(knot.left_offset, make_pointer(knot_pointer, LEFT_OFFSET_KEY));
    serializer.set_value(knot.left_value, make_pointer(knot_pointer, LEFT_VALUE_KEY));
    serializer.set_value(knot.right_offset, make_pointer(knot_pointer, RIGHT_OFFSET_KEY));
    serializer.set_value(knot.right_value, make_pointer(knot_pointer, RIGHT_VALUE_KEY));
  }
  serializer.end_array();
}

void Track::deserialize(AbstractDeserializer& deserializer, const Pointer& pointer)
{
  m_property_key = deserializer.get_string(make_pointer(pointer, PROPERTY_KEY_KEY));
  m_owner_id = deserializer.get_size_t(make_pointer(pointer, OWNER_KEY));
  deserializer.register_reference_polisher(*this);
  const std::string type = deserializer.get_string(make_pointer(pointer, TYPE_KEY));

  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const std::size_t n = deserializer.array_size(knots_pointer);
  for (std::size_t i = 0; i < n; ++i) {
    const auto knot_pointer = make_pointer(knots_pointer, i);
    Knot knot(deserializer.get(make_pointer(knot_pointer, VALUE_KEY), type));
    knot.left_offset = deserializer.get_int(make_pointer(knot_pointer, LEFT_OFFSET_KEY));
    knot.left_value = deserializer.get(make_pointer(knot_pointer, LEFT_VALUE_KEY), type);
    knot.right_offset = deserializer.get_int(make_pointer(knot_pointer, RIGHT_OFFSET_KEY));
    knot.right_value = deserializer.get(make_pointer(knot_pointer, RIGHT_VALUE_KEY), type);
    const int frame = deserializer.get_int(make_pointer(knot_pointer, FRAME_KEY));
    m_knots.insert(std::pair(frame, knot));
  }
}

void Track::set_owner(AbstractPropertyOwner &owner, const std::string &property_key)
{
  assert(m_owner == nullptr && m_property_key.empty());
  m_owner = &owner;
  m_property_key = property_key;
}

void Track::record(int frame, Property &property)
{
  assert(!has_keyframe(frame));
  m_knots.insert(std::pair(frame, property.variant_value()));
  Q_EMIT track_changed();
}

void Track::remove_keyframe(int frame)
{
  const auto it = m_knots.find(frame);
  assert(it != m_knots.end());
  m_knots.erase(it);
  Q_EMIT track_changed();
}

Track::Knot &Track::knot_at(int frame)
{
  return m_knots.at(frame);
}

std::vector<int> Track::key_frames() const
{
  return ::transform<int, std::vector>(m_knots, [](const auto& p) {
    return p.first;
  });
}

const Track::Knot &Track::knot_at(int frame) const
{
  return m_knots.at(frame);
}

std::string Track::type() const
{
  static const std::string property_suffix = "Property";
  const std::string type = property().type();
  assert(type.size() > property_suffix.size());
  return type.substr(0, type.size() - property_suffix.size());
}

Property &Track::property() const
{
  return *m_owner->property(m_property_key);
}

Track::Knot::Knot(const variant_type &value)
  : value(value), left_offset(0), left_value(value), right_offset(0), right_value(value)
{
}

}  // namespace omm
