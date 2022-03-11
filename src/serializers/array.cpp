#include "serializers/array.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"

namespace omm::serialization
{

DeserializationArray::DeserializationArray(DeserializerWorker& parent, const std::size_t size)
    : Array(parent)
    , m_size(size)
{
}

std::size_t DeserializationArray::size() const
{
  return m_size;
}

}  // namespace omm::serialization
