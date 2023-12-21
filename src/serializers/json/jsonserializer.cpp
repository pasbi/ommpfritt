#include "serializers/json/jsonserializer.h"

#include "serializers/json/jsonserializerworker.h"
#include <nlohmann/json.hpp>

namespace omm::serialization
{

JSONSerializer::JSONSerializer(nlohmann::json& json) : m_json(json)
{
}

std::unique_ptr<SerializerWorker> JSONSerializer::worker()
{
  return std::make_unique<JSONSerializerWorker>(m_json);
}

}  // namespace omm::serialization
