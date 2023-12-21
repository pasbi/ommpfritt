#include "serializers/json/jsondeserializer.h"

#include "serializers/json/jsondeserializerworker.h"
#include <nlohmann/json.hpp>

namespace omm::serialization
{

JSONDeserializer::JSONDeserializer(const nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<DeserializerWorker> JSONDeserializer::worker()
{
  return std::make_unique<JSONDeserializerWorker>(*this, m_json);
}

}  // namespace omm::serialization
