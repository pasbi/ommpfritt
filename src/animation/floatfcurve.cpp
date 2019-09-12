#include "animation/floatfcurve.h"

namespace omm
{

double FloatFCurve::interpolate(int frame) const
{
  return 0.0;
}

std::size_t FloatFCurve::count() const
{
  return m_keyvalues.size();
}

void FloatFCurve::
deserialize_keyvalue(AbstractDeserializer &, std::size_t i, const Serializable::Pointer &pointer)
{

}

void FloatFCurve::
serialize_keyvalue(AbstractSerializer &, std::size_t i, const Serializable::Pointer &pointer) const
{

}



}  // namespace
