#include "serializers/jsonserializer.h"

#include <glog/logging.h>
#include <typeinfo>

#include "geometry/objecttransformation.h"
#include "aspects/serializable.h"

namespace
{

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
      return value;
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
      std::string("Cannot find '") + std::string(pointer) + "'.");
  } catch (const nlohmann::json::parse_error& json_exception) {
    throw omm::AbstractDeserializer::DeserializeError(
      std::string("Invalid pointer '") + std::string(pointer) + "'.");
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
  m_store[ptr(pointer)] = value;
}

void JSONSerializer::set_value(const std::string& value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = value;
}

void JSONSerializer::set_value(const ObjectTransformation& value, const Pointer& pointer)
{
  auto& matrix_json = m_store[ptr(pointer)];
  for (size_t i_row = 0; i_row < ObjectTransformation::N_ROWS; ++i_row) {
    auto& row = matrix_json[i_row];
    for (size_t i_col = 0; i_col < ObjectTransformation::N_COLS; ++i_col) {
      const auto matrix = value.to_mat();
      row[i_col] = matrix(i_row, i_col);
    }
  }
}

void JSONSerializer::set_value(const std::size_t id, const Pointer& pointer)
{
  m_store[ptr(pointer)] = id;
}

void JSONSerializer::set_value(const Color& color, const Pointer& pointer)
{
  m_store[ptr(pointer)] = { color.red(), color.green(), color.blue(), color.alpha() };
}

std::string JSONSerializer::type() const
{
  return "JSONSerializer";
}



JSONDeserializer::JSONDeserializer(std::istream& istream)
  : AbstractDeserializer(istream)
{
  istream >> m_store;
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

ObjectTransformation JSONDeserializer::get_object_transformation(const Pointer& pointer)
{
  ObjectTransformation::Mat mat;
  for (size_t i_row = 0; i_row < ObjectTransformation::N_ROWS; ++i_row) {
    for (size_t i_col = 0; i_col < ObjectTransformation::N_COLS; ++i_col) {
      const auto element_pointer = Serializable::make_pointer(pointer, i_row, i_col);
      mat(i_row, i_col) = get_t<double>(m_store, element_pointer);
    }
  }
  return ObjectTransformation(mat);
}

std::size_t JSONDeserializer::get_size_t(const Pointer& pointer)
{
  return get_t<std::size_t>(m_store, pointer);
}

std::string JSONDeserializer::type() const
{
  return "JSONDeserializer";
}

}  // namespace omm
