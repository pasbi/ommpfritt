#include "serializers/json/jsondeserializerworker.h"

#include "external/json.hpp"
#include "geometry/polarcoordinates.h"
#include "serializers/abstractdeserializer.h"
#include "serializers/json/common.h"


namespace
{

double get_double(const nlohmann::json& json_val)
{
  using omm::serialization::inf_value;
  using omm::serialization::neg_inf_value;
  if (json_val.is_number()) {
    return json_val;
  } else if (json_val.is_string()) {
    const std::string value = json_val;
    if (value == inf_value) {
      return std::numeric_limits<double>::infinity();
    } else if (value == neg_inf_value) {
      return -std::numeric_limits<double>::infinity();
    } else {
      const std::string msg = std::string("Expected '") + inf_value + "' or '" + neg_inf_value
                              + "' but got '" + value + "'.";
      throw omm::serialization::AbstractDeserializer::DeserializeError(msg);
    }
  } else {
    throw omm::serialization::AbstractDeserializer::DeserializeError("invalid type.");
  }
}

template<typename T>
T get_t(const nlohmann::json& value)
{
  try {
    if constexpr (std::is_same_v<T, double>) {
      // get inf properly
      return get_double(value);
    } else if constexpr (std::is_same_v<T, std::vector<double>>) {
      return util::transform<std::vector>(static_cast<std::vector<nlohmann::json>>(value), get_double);
    } else if constexpr (std::is_same_v<T, QString>) {
      return QString::fromStdString(value);
    } else {
      return value;
    }
  } catch (const nlohmann::json::type_error& convert_exception) {
    std::ostringstream message;
    message << "Failed to convert\n";
    message << value << "\n";
    message << "to '" << typeid(T).name() << "'.";
    throw omm::serialization::AbstractDeserializer::DeserializeError(message.str());
  }
}

}  // namespace


namespace omm::serialization
{

JSONDeserializerWorker::JSONDeserializerWorker(AbstractDeserializer& deserializer, const nlohmann::json& value)
    : DeserializerWorker(deserializer)
    , m_value(value)
{
}

int JSONDeserializerWorker::get_int()
{
  return get_t<int>(m_value);
}

bool JSONDeserializerWorker::get_bool()
{
  return get_t<bool>(m_value);
}

double JSONDeserializerWorker::get_double()
{
  return get_t<double>(m_value);
}

QString JSONDeserializerWorker::get_string()
{
  return get_t<QString>(m_value);
}

std::size_t JSONDeserializerWorker::get_size_t()
{
  return get_t<std::size_t>(m_value);
}

TriggerPropertyDummyValueType JSONDeserializerWorker::get_trigger_dummy_value()
{
  return {};
}

std::unique_ptr<DeserializationArray> JSONDeserializerWorker::start_array()
{
  if (!m_value.is_array()) {
    throw AbstractDeserializer::DeserializeError{"Expected Array"};
  }
  return std::make_unique<DeserializationArray>(*this, m_value.size());
}

std::unique_ptr<DeserializerWorker> JSONDeserializerWorker::sub(const std::string& key)
{
  if (!m_value.is_object()) {
    throw AbstractDeserializer::DeserializeError{"Attempt to access non-object value by key"};
  }
  try {
    return std::make_unique<JSONDeserializerWorker>(deserializer(), m_value.at(key));
  } catch (const nlohmann::json::out_of_range& e) {
    throw AbstractDeserializer::DeserializeError{e.what()};
  }
}

std::unique_ptr<DeserializerWorker> JSONDeserializerWorker::sub(const std::size_t i)
{
  if (!m_value.is_array()) {
    throw AbstractDeserializer::DeserializeError{"Attempt to access non-array value by index"};
  }
  try {
    return std::make_unique<JSONDeserializerWorker>(deserializer(), m_value[i]);
  } catch (const nlohmann::json::out_of_range& e) {
    throw AbstractDeserializer::DeserializeError{e.what()};
  }
}

}  // namespace omm::serialization
