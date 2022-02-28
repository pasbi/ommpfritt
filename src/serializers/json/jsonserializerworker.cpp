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
