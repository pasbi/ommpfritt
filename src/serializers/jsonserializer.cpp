#include "serializers/jsonserializer.h"

#include <glog/logging.h>

#include "objects/objecttransformation.h"
#include "objects/objecttransformation.h"

namespace
{

auto ptr(const omm::Serializable::Pointer& pointer)
{
  LOG(INFO) << "pointer: " << pointer;
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

}

void JSONSerializer::end_array()
{

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
  m_store[ptr(pointer)] = "otr";
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
  // TODO: load store from istream
  AbstractDeserializer::deserialize(scene, istream);
  istream >> m_store;
  return istream;
}


size_t JSONDeserializer::array_size(const Pointer& pointer)
{
  return 0;
}

int  JSONDeserializer::get_int(const Pointer& pointer)
{
  return 0;
}

double  JSONDeserializer::get_double(const Pointer& pointer)
{
  return 0.0;
}

std::string  JSONDeserializer::get_string(const Pointer& pointer)
{
  return "";
}

ObjectTransformation JSONDeserializer::get_object_transformation(const Pointer& pointer)
{
  return ObjectTransformation();
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
