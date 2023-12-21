#include "serializers/bin/bindeserializer.h"

#include "serializers/bin/bindeserializerworker.h"
#include <nlohmann/json.hpp>

namespace omm::serialization
{

BinDeserializer::BinDeserializer(QDataStream& stream)
    : m_stream(stream)
{
}

std::unique_ptr<DeserializerWorker> BinDeserializer::worker()
{
  return std::make_unique<BinDeserializerWorker>(*this, m_stream);
}

}  // namespace omm::serialization
