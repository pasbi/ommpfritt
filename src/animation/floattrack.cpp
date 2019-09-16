#include "animation/floattrack.h"
#include "properties/floatproperty.h"

namespace omm
{

double FloatTrack::interpolate(int frame) const
{
//  AbstractTrack::
  return 0.0;
}

std::size_t FloatTrack::count() const
{
  return m_keyvalues.size();
}

bool FloatTrack::has_key_at(int frame) const
{
  return m_keyvalues.find(frame) != m_keyvalues.end();
}

void FloatTrack::record(int frame, Property &property)
{
  assert(!has_key_at(frame));
  assert(AbstractTrack::map_property_to_track_type(property.type()) == type());
  m_keyvalues.insert(std::pair(frame, static_cast<FloatProperty&>(property).value()));
  Q_EMIT track_changed();
}

void FloatTrack::remove_key_at(int frame)
{
  auto it = m_keyvalues.find(frame);
  assert(it != m_keyvalues.end());
  m_keyvalues.erase(it);
  Q_EMIT track_changed();
}

void FloatTrack::
deserialize_keyvalue(AbstractDeserializer &, std::size_t i, const Serializable::Pointer &pointer)
{
}

void FloatTrack::
serialize_keyvalue(AbstractSerializer &, std::size_t i, const Serializable::Pointer &pointer) const
{
}

FloatTrack::KeyValue::KeyValue(double value)
  : value(value), left_offset(0), left_value(value), right_offset(0), right_value(value)
{
}



}  // namespace
