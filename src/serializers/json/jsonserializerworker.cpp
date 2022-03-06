#include "serializers/json/jsonserializerworker.h"

#include "external/json.hpp"
#include "geometry/polarcoordinates.h"
#include "serializers/abstractserializer.h"
#include "serializers/json/common.h"

namespace
{

nlohmann::json set_double(double value)
{
  // unfortunately, json cannot store infinity values, hence the workaround.
  if (value == std::numeric_limits<double>::infinity()) {
    return omm::serialization::inf_value;
  } else if (value == -std::numeric_limits<double>::infinity()) {
    return omm::serialization::neg_inf_value;
  } else {
    return value;
  }
}

}  // namespace

namespace omm::serialization
{

std::unique_ptr<SerializationArray> JSONSerializerWorker::start_array([[maybe_unused]] std::size_t size)
{
  m_value = nlohmann::json::value_type::array();
  return std::make_unique<SerializationArray>(*this);
}

JSONSerializerWorker::JSONSerializerWorker(nlohmann::json& value)
    : m_value(value)
{
}

void JSONSerializerWorker::set_value(int value)
{
  m_value = value;
}

void JSONSerializerWorker::set_value(bool value)
{
  m_value = value;
}

void JSONSerializerWorker::set_value(double value)
{
  m_value = ::set_double(value);
}

void JSONSerializerWorker::set_value(const QString& value)
{
  m_value = value.toStdString();
}

void JSONSerializerWorker::set_value(const std::size_t id)
{
  m_value = id;
}

void JSONSerializerWorker::set_value(const TriggerPropertyDummyValueType&)
{
}

std::unique_ptr<SerializerWorker> JSONSerializerWorker::sub(const std::string& key)
{
  if (m_value.is_null()) {
    m_value = nlohmann::json::object();
  }
  if (!m_value.is_object()) {
    throw AbstractSerializer::SerializeError{"Attempt to access non-object value by key"};
  }
  try {
    return std::make_unique<JSONSerializerWorker>(m_value[key]);
  } catch (const nlohmann::json::out_of_range&) {
    throw AbstractSerializer::SerializeError{"Attempt to access non-existing key: " + key};
  }
}

std::unique_ptr<SerializerWorker> JSONSerializerWorker::sub(const std::size_t i)
{
  if (!m_value.is_array()) {
    throw AbstractSerializer::SerializeError{"Attempt to access non-array value by index"};
  }
  try {
    return std::make_unique<JSONSerializerWorker>(m_value[i]);
  } catch (const nlohmann::json::out_of_range&) {
    throw AbstractSerializer::SerializeError{"Attempt to access non-existing index: " + std::to_string(i)};
  }
}

}  // namespace omm::serialization
