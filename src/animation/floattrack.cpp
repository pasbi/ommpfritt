#include "animation/floattrack.h"

namespace omm
{

double FloatTrack::interpolate(int frame) const
{
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

void FloatTrack::
deserialize_keyvalue(AbstractDeserializer &, std::size_t i, const Serializable::Pointer &pointer)
{

}

void FloatTrack::
serialize_keyvalue(AbstractSerializer &, std::size_t i, const Serializable::Pointer &pointer) const
{

}



}  // namespace
