#include "serializers/deserializerworker.h"
#include "serializers/abstractdeserializer.h"
#include "geometry/polarcoordinates.h"
#include "geometry/vec2.h"

namespace
{

template<typename T> omm::Vec2<T> deserialize_vec2(omm::serialization::DeserializerWorker& worker)
{
  const auto vs = worker.get<std::vector<T>>();
  if (vs.size() != 2) {
    throw omm::serialization::AbstractDeserializer::DeserializeError("Expected vector of size 2.");
  }
  return omm::Vec2{vs};
}

}  // namepsace

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

template<> void DeserializerWorker::get<Vec2f>(Vec2f& v)
{
  v = deserialize_vec2<double>(*this);
}

template<> void DeserializerWorker::get<Vec2i>(Vec2i& v)
{
  v = deserialize_vec2<int>(*this);
}

template<> void DeserializerWorker::get<bool>(bool& v)
{
  v = get_bool();
}

template<> void DeserializerWorker::get<int>(int& v)
{
  v = get_int();
}

template<> void DeserializerWorker::get<double>(double& v)
{
  v = get_double();
}

template<> void DeserializerWorker::get<QString>(QString& v)
{
  v = get_string();
}

template<> void DeserializerWorker::get<std::size_t>(std::size_t& v)
{
  v = get_size_t();
}

template<> void DeserializerWorker::get<TriggerPropertyDummyValueType>(TriggerPropertyDummyValueType&)
{
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
