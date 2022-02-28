#include "serializers/deserializerworker.h"
#include "geometry/polarcoordinates.h"

namespace  omm::serialization
{

DeserializerWorker::DeserializerWorker(AbstractDeserializer& deserializer)
    : m_deserializer(deserializer)
{
}

AbstractDeserializer& DeserializerWorker::deserializer() const
{
  return m_deserializer;
}

template<> bool DeserializerWorker::get<bool>()
{
  return get_bool();
}

template<> int DeserializerWorker::get<int>()
{
  return get_int();
}

template<> double DeserializerWorker::get<double>()
{
  return get_double();
}

template<> QString DeserializerWorker::get<QString>()
{
  return get_string();
}

template<> std::size_t DeserializerWorker::get<std::size_t>()
{
  return get_size_t();
}

template<> Color DeserializerWorker::get<Color>()
{
  return get_color();
}

template<> Vec2f DeserializerWorker::get<Vec2f>()
{
  return get_vec2f();
}

template<> Vec2i DeserializerWorker::get<Vec2i>()
{
  return get_vec2i();
}

template<> PolarCoordinates DeserializerWorker::get<PolarCoordinates>()
{
  return get_polarcoordinates();
}

template<> SplineType DeserializerWorker::get<SplineType>()
{
  return get_spline();
}

template<> TriggerPropertyDummyValueType DeserializerWorker::get<TriggerPropertyDummyValueType>()
{
  return {};
}

variant_type DeserializerWorker::get(const QString& type)
{
  if (type == "Bool") {
    return get<bool>();
  } else if (type == "Integer") {
    return get<int>();
  } else if (type == "Float") {
    return get<double>();
  } else if (type == "String") {
    return get<QString>();
  } else if (type == "Options") {
    return get<std::size_t>();
  } else if (type == "Color") {
    return get<Color>();
  } else if (type == "FloatVector") {
    return get<Vec2f>();
  } else if (type == "IntegerVector") {
    return get<Vec2i>();
  } else if (type == "SplineType") {
    return get<SplineType>();
  } else if (type == "Reference") {
    Q_UNREACHABLE();
    // Reference values must be treated specially.
    // See ReferenceProperty for details.
    return nullptr;
  } else if (type == "Trigger") {
    return get<TriggerPropertyDummyValueType>();
  } else {
    LERROR << "Unknown variant type: '" << type << "'.";
    Q_UNREACHABLE();
  }
}


}  // namespace omm::serialization
