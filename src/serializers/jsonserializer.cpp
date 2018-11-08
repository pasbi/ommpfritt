#include "serializers/jsonserializer.h"

#include <glog/logging.h>

#include "objects/objecttransformation.h"
#include "objects/objecttransformation.h"

namespace
{

auto ptr(const omm::Serializable::Pointer& pointer)
{
  return nlohmann::json::json_pointer(pointer);
}

}  // namespace

namespace omm
{

std::ostream& JSONSerializer::serialize(const Scene& scene, std::ostream& ostream)
{
  AbstractSerializer::serialize(scene, ostream);
#ifdef NDEBUG
  ostream << m_store;
#else
  ostream << std::setw(4) << m_store;
#endif
  return ostream;
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
      row[i_col] = value(i_row, i_col);
    }
  }
}

void JSONSerializer::set_value(const Object* value, const Pointer& pointer)
{
  m_store[ptr(pointer)] = nullptr;
}

std::string JSONSerializer::type() const
{
  return "JSONSerializer";
}

std::istream& JSONDeserializer::deserialize(Scene& scene, std::istream& istream)
{
  istream >> m_store;
  AbstractDeserializer::deserialize(scene, istream);
  return istream;
}


size_t JSONDeserializer::array_size(const Pointer& pointer)
{
  const auto array = m_store[ptr(pointer)];
  assert(array.is_array() || array.is_null());
  return array.size();
}

int JSONDeserializer::get_int(const Pointer& pointer)
{
  return m_store[ptr(pointer)];
}

double JSONDeserializer::get_double(const Pointer& pointer)
{
  return m_store[ptr(pointer)];
}

std::string JSONDeserializer::get_string(const Pointer& pointer)
{
  return m_store[ptr(pointer)];
}

ObjectTransformation JSONDeserializer::get_object_transformation(const Pointer& pointer)
{
  const auto& matrix_json = m_store[ptr(pointer)];
  ObjectTransformation transformation;
  assert(matrix_json.is_array() && matrix_json.size() == ObjectTransformation::N_ROWS);
  for (size_t i_row = 0; i_row < ObjectTransformation::N_ROWS; ++i_row) {
    const auto& row = matrix_json[i_row];
    assert(matrix_json.is_array() && matrix_json.size() == ObjectTransformation::N_COLS);
    for (size_t i_col = 0; i_col < ObjectTransformation::N_COLS; ++i_col) {
      transformation(i_row, i_col) = row[i_col];
    }
  }
  return transformation;
}

Object* JSONDeserializer::get_object_reference(const Pointer& pointer)
{
  return nullptr;
}

std::string JSONDeserializer::type() const
{
  return "JSONDeserializer";
}

}  // namespace omm
