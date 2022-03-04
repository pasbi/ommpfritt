#include "serializers/json/jsondeserializer.h"

#include "external/json.hpp"
#include "serializers/json/jsondeserializerworker.h"

namespace omm::serialization
{

JSONDeserializer::JSONDeserializer(const nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<DeserializerWorker> JSONDeserializer::sub(const std::string& key)
{
  return std::make_unique<JSONDeserializerWorker>(*this, m_json[key]);
}

}  // namespace omm::serialization
