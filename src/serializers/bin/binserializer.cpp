#include "serializers/bin/binserializer.h"
#include "serializers/bin/binserializerworker.h"

namespace omm::serialization
{

BinSerializer::BinSerializer(QDataStream& stream) : m_stream(stream)
{
}

std::unique_ptr<SerializerWorker> BinSerializer::worker()
{
  return std::make_unique<BinSerializerWorker>(m_stream);
}

}  // namespace omm
