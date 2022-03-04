#include "serializers/json/jsonserializer.h"

#include "external/json.hpp"
#include "serializers/json/jsonserializerworker.h"

namespace omm::serialization
{

JSONSerializer::JSONSerializer(nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<SerializerWorker> JSONSerializer::sub(const std::string& key)
{
  return std::make_unique<JSONSerializerWorker>(m_json[key]);
}

}  // namespace omm
