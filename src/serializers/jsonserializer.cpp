#include "serializers/jsonserializer.h"

#include <glog/logging.h>
#include <typeinfo>

#include "aspects/serializable.h"

namespace
{

constexpr auto inf_value = "inf";
constexpr auto neg_inf_value = "-inf";

double get_double(const nlohmann::json& json_val)
{
  using namespace std::string_literals;
  if (json_val.is_number()) {
    return json_val;
  } else if (json_val.is_string()) {
    const std::string value = json_val;
    if (value == inf_value) {
      return std::numeric_limits<double>::infinity();
    } else if (value == neg_inf_value) {
      return -std::numeric_limits<double>::infinity();
    } else {
      const std::string msg = "Expected '"s + inf_value + "' or '"
                            + neg_inf_value + "' but got '" + value + "'.";
      throw omm::AbstractDeserializer::DeserializeError(msg);
    }
  } else {
    throw omm::AbstractDeserializer::DeserializeError("invalid type.");
  }
  return std::numeric_limits<double>::signaling_NaN();
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

auto ptr(const omm::Serializable::Pointer& pointer)
{
  return nlohmann::json::json_pointer(pointer);
}

template<typename T>
T get_t(const nlohmann::json& json, const nlohmann::json::json_pointer& pointer)
{
  try {
    const auto value = json.at(pointer);
    try {
      if constexpr (std::is_same_v<T, double>) {
        // get inf properly
        return get_double(value);
      } else {
        return value;
      }
    } catch (const nlohmann::json::type_error& convert_exception) {
      std::stringstream message;
      message << "Failed to convert\n";
      message << value << "\n";
      message << "at '" << pointer << "'\n";
      message << "to '" << typeid(T).name() << "'.";
      throw omm::AbstractDeserializer::DeserializeError(message.str());
    }
  } catch (const nlohmann::json::out_of_range& json_exception) {
    throw omm::AbstractDeserializer::DeserializeError(
      "Cannot find '" + std::string(pointer) + "'.");
  } catch (const nlohmann::json::parse_error& json_exception) {
    throw omm::AbstractDeserializer::DeserializeError(
      "Invalid pointer '" + std::string(pointer) + "'.");
  }
}

template<typename T, typename PointerT>
T get_t(const nlohmann::json& json, const PointerT& pointer)
{
  return get_t<T>(json, nlohmann::json::json_pointer(pointer));
}

}  // namespace

namespace omm
{

JSONSerializer::JSONSerializer(std::ostream& ostream)
  : AbstractSerializer(ostream)
  , m_ostream(ostream)
{
}

JSONSerializer::~JSONSerializer()
{
#ifdef NDEBUG
  m_ostream << m_store;
#else
  m_ostream << std::setw(4) << m_store;
#endif
}

void JSONSerializer::start_array(size_t size, const Pointer& pointer)
{
  Q_UNUSED(size);
  Q_UNUSED(pointer);
  // no action required
}

void JSONSerializer::end_array()
{
  // no action required
}

void JSONSerializer::set_value(int value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = value;
}

void JSONSerializer::set_value(bool value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = value;
}

void JSONSerializer::set_value(double value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = ::set_double(value);
}

void JSONSerializer::set_value(const std::string& value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = value;
}

void JSONSerializer::set_value(const std::size_t id, const Pointer& pointer)
{
  m_store[ptr(pointer)] = id;
}

void JSONSerializer::set_value(const Color& color, const Pointer& pointer)
{
  m_store[ptr(pointer)] = { color.red(), color.green(), color.blue(), color.alpha() };
}

void JSONSerializer::set_value(const arma::vec2& value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = { set_double(value[0]), set_double(value[1]) };
}

void JSONSerializer::set_value(const arma::ivec2& value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = { value[0], value[1] };
}

void JSONSerializer::set_value(const PolarCoordinates& value, const Pointer& pointer)
{
  set_value(arma::vec2{ value.argument, value.magnitude }, pointer);
}

std::string JSONSerializer::type() const { return "JSONSerializer"; }



JSONDeserializer::JSONDeserializer(std::istream& istream)
  : AbstractDeserializer(istream)
{
  try {
    istream >> m_store;
  } catch (const nlohmann::detail::parse_error& error) {
    throw omm::AbstractDeserializer::DeserializeError(error.what());
  }
}

size_t JSONDeserializer::array_size(const Pointer& pointer)
{
  const auto array = m_store[ptr(pointer)];
  assert(array.is_array() || array.is_null());
  return array.size();
}

int JSONDeserializer::get_int(const Pointer& pointer)
{
  return get_t<int>(m_store, pointer);
}

bool JSONDeserializer::get_bool(const Pointer& pointer)
{
  return get_t<bool>(m_store, pointer);
}

double JSONDeserializer::get_double(const Pointer& pointer)
{
  return get_t<double>(m_store, pointer);
}

std::string JSONDeserializer::get_string(const Pointer& pointer)
{
  return get_t<std::string>(m_store, pointer);
}

Color JSONDeserializer::get_color(const Pointer& pointer)
{
  arma::vec4 color;
  for (size_t i = 0; i < color.n_elem; ++i) {
    const auto element_pointer = Serializable::make_pointer(pointer, i);
    color[i] = get_t<double>(m_store, element_pointer);
  }
  return color;
}

std::size_t JSONDeserializer::get_size_t(const Pointer& pointer)
{
  return get_t<std::size_t>(m_store, pointer);
}

arma::vec2 JSONDeserializer::get_vec2(const Pointer& pointer)
{
  try {
    const double x = ::get_double(m_store.at(ptr(pointer + "/0")));
    const double y = ::get_double(m_store.at(ptr(pointer + "/1")));
    return arma::vec2{ x, y };
  } catch (const nlohmann::json::out_of_range& json_exception) {
    throw omm::AbstractDeserializer::DeserializeError(
      "Cannot find vector of size two at '" + std::string(pointer) + "'.");
  } catch (const nlohmann::json::parse_error& json_exception) {
    throw omm::AbstractDeserializer::DeserializeError(
      "Invalid pointer '" + std::string(pointer) + "'.");
  }
  return arma::vec2{};
}

arma::ivec2 JSONDeserializer::get_ivec2(const Pointer& pointer)
{
  const auto vec2 = get_t<std::vector<int>>(m_store, pointer);
  if (vec2.size() != 2) {
    throw omm::AbstractDeserializer::DeserializeError("Expected vector of size 2.");
  }

  return arma::ivec2{ vec2[0], vec2[1] };
}

PolarCoordinates JSONDeserializer::get_polarcoordinates(const Pointer& pointer)
{
  const auto pair = get_vec2(pointer);
  return PolarCoordinates(pair[0], pair[1]);
}

std::string JSONDeserializer::type() const { return "JSONDeserializer"; }

}  // namespace omm
