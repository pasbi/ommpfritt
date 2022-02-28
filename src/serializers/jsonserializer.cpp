#include "serializers/jsonserializer.h"

#include "aspects/serializable.h"
#include "common.h"
#include "logging.h"
#include "geometry/polarcoordinates.h"
#include "serializers/abstractdeserializer.h"
#include "serializers/abstractserializer.h"
#include "geometry/polarcoordinates.h"
#include <iomanip>
#include <typeinfo>

namespace
{
constexpr auto inf_value = "inf";
constexpr auto neg_inf_value = "-inf";

double get_double(const nlohmann::json& json_val)
{
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

nlohmann::json set_double(double value)
{
  // unfortunately, json cannot store infinity values, hence the workaround.
  if (value == std::numeric_limits<double>::infinity()) {
    return inf_value;
  } else if (value == -std::numeric_limits<double>::infinity()) {
    return neg_inf_value;
  } else {
    return value;
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

std::unique_ptr<SerializationArray> JSONSerializerWorker::start_array([[maybe_unused]] std::size_t size)
{
  class JSONArray : public SerializationArray
  {
  public:
    using SerializationArray::SerializationArray;
    SerializerWorker& next() override
    {
      m_current = m_parent.sub(m_next_index);
      m_next_index += 1;
      return *m_current;
    }
  };

  m_value = nlohmann::json::value_type::array();
  return std::make_unique<JSONArray>(*this);
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

void JSONSerializerWorker::set_value(const Color& color)
{
  auto& name = m_value["name"];
  auto& rgba = m_value["rgba"];
  if (color.model() == Color::Model::Named) {
    name = color.name().toStdString();
    rgba = {0.0, 0.0, 0.0, 0.0};
  } else {
    name = "";
    rgba = color.components(Color::Model::RGBA);
  }
}

void JSONSerializerWorker::set_value(const Vec2f& value)
{
  m_value = {set_double(value[0]), set_double(value[1])};
}

void JSONSerializerWorker::set_value(const Vec2i& value)
{
  m_value = {value[0], value[1]};
}

void JSONSerializerWorker::set_value(const PolarCoordinates& value)
{
  set_value(Vec2f(value.argument, value.magnitude));
}

void JSONSerializerWorker::set_value(const TriggerPropertyDummyValueType&)
{
}

void JSONSerializerWorker::set_value(const SplineType& spline)
{
  m_value = nlohmann::json::value_type::array();
  for (const auto& [t, knot] : spline.knots) {
    m_value.push_back({t, knot.value, knot.left_offset, knot.right_offset});
  }
}

std::unique_ptr<SerializerWorker> JSONSerializerWorker::sub(const std::string& key)
{
  if (!m_value.is_object()) {
    throw omm::serialization::AbstractSerializer::SerializeError{"Attempt to access non-object value by key"};
  }
  try {
    return std::make_unique<JSONSerializerWorker>(m_value[key]);
  } catch (const nlohmann::json::out_of_range&) {
    throw omm::serialization::AbstractSerializer::SerializeError{"Attempt to access non-existing key: " + key};
  }
}

std::unique_ptr<SerializerWorker> JSONSerializerWorker::sub(const std::size_t i)
{
  if (!m_value.is_array()) {
    throw omm::serialization::AbstractSerializer::SerializeError{"Attempt to access non-array value by index"};
  }
  try {
    return std::make_unique<JSONSerializerWorker>(m_value[i]);
  } catch (const nlohmann::json::out_of_range&) {
    throw omm::serialization::AbstractSerializer::SerializeError{"Attempt to access non-existing index: " + std::to_string(i)};
  }
}

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

Color JSONDeserializerWorker::get_color()
{
  try {
    const auto v = get_t<std::vector<double>>(m_value["rgba"]);
    const auto n = sub("name")->get_string();
    if (n.isEmpty()) {
      return Color(Color::Model::RGBA, {v.at(0), v.at(1), v.at(2), v.at(3)});
    } else {
      return Color(n);
    }
  } catch (std::out_of_range&) {
    throw omm::serialization::AbstractDeserializer::DeserializeError("Expected vector of size 4.");
  }
}

std::size_t JSONDeserializerWorker::get_size_t()
{
  return get_t<std::size_t>(m_value);
}

Vec2f JSONDeserializerWorker::get_vec2f()
{
  try {
    return Vec2f(get_t<std::vector<double>>(m_value));
  } catch (std::out_of_range&) {
    throw AbstractDeserializer::DeserializeError("Expected vector of size 2.");
  }
}

Vec2i JSONDeserializerWorker::get_vec2i()
{
  try {
    return Vec2i(get_t<std::vector<int>>(m_value));
  } catch (std::out_of_range&) {
    throw AbstractDeserializer::DeserializeError("Expected vector of size 2.");
  }
}

PolarCoordinates JSONDeserializerWorker::get_polarcoordinates()
{
  const auto pair = get_vec2f();
  return PolarCoordinates(pair[0], pair[1]);
}

TriggerPropertyDummyValueType JSONDeserializerWorker::get_trigger_dummy_value()
{
  return {};
}

SplineType JSONDeserializerWorker::get_spline()
{
  SplineType::knot_map_type map;
  for (const auto& item : m_value) {
    map.insert({item.at(0), SplineType::Knot(item.at(1), item.at(2), item.at(3))});
  }

  return SplineType(map);
}

std::unique_ptr<DeserializationArray> JSONDeserializerWorker::start_array()
{
  class JSONArray : public DeserializationArray
  {
  public:
    explicit JSONArray(JSONDeserializerWorker& parent, const std::size_t size)
        : DeserializationArray(parent)
        , m_size(size)
    {
    }

    DeserializerWorker& next() override
    {
      m_current = m_parent.sub(m_next_index);
      m_next_index += 1;
      return *m_current;
    }

    std::size_t size() const override
    {
      return m_size;
    }

  private:
    const std::size_t m_size;
  };

  if (!m_value.is_array()) {
    throw omm::serialization::AbstractDeserializer::DeserializeError{"Expected Array"};
  }
  return std::make_unique<JSONArray>(*this, m_value.size());
}

std::unique_ptr<DeserializerWorker> JSONDeserializerWorker::sub(const std::string& key)
{
  if (!m_value.is_object()) {
    throw omm::serialization::AbstractDeserializer::DeserializeError{"Attempt to access non-object value by key"};
  }
  try {
    return std::make_unique<JSONDeserializerWorker>(deserializer(), m_value[key]);
  } catch (const nlohmann::json::out_of_range&) {
    throw omm::serialization::AbstractDeserializer::DeserializeError{"Attempt to access non-existing key: " + key};
  }
}

std::unique_ptr<DeserializerWorker> JSONDeserializerWorker::sub(const std::size_t i)
{
  if (!m_value.is_array()) {
    throw omm::serialization::AbstractDeserializer::DeserializeError{"Attempt to access non-array value by index"};
  }
  try {
    return std::make_unique<JSONDeserializerWorker>(deserializer(), m_value[i]);
  } catch (const nlohmann::json::out_of_range&) {
    throw omm::serialization::AbstractDeserializer::DeserializeError{"Attempt to access non-existing index: " + std::to_string(i)};
  }
}

JSONSerializer::JSONSerializer(nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<SerializerWorker> JSONSerializer::sub(const std::string& key)
{
  return std::make_unique<JSONSerializerWorker>(m_json[key]);
}

JSONDeserializer::JSONDeserializer(const nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<DeserializerWorker> JSONDeserializer::sub(const std::string& key)
{
  return std::make_unique<JSONDeserializerWorker>(*this, m_json[key]);
}

}  // namespace omm
