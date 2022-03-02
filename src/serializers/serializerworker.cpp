#include "serializers/serializerworker.h"
#include "aspects/propertyowner.h"


namespace
{

template<typename T> void serialize_vec2(omm::serialization::SerializerWorker& worker, const omm::Vec2<T>& value)
{
  worker.set_value(value.to_stdvec());
}

}  // namepsace


namespace omm::serialization
{

void SerializerWorker::set_value(const AbstractPropertyOwner* ref)
{
  set_value(ref == nullptr ? 0 : ref->id());
}

void SerializerWorker::set_value(const Vec2f& value)
{
  serialize_vec2(*this, value);
}

void SerializerWorker::set_value(const Vec2i& value)
{
  serialize_vec2(*this, value);
}

void SerializerWorker::set_value(const variant_type& variant)
{
  std::visit([this](auto&& arg) { set_value(arg); }, variant);
}

}  // namespace omm::serialization
