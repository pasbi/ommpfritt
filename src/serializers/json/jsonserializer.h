#pragma once

#include "serializers/abstractserializer.h"
#include <nlohmann/json_fwd.hpp>

namespace omm::serialization
{

class JSONSerializer : public AbstractSerializer
{
public:
  explicit JSONSerializer(nlohmann::json& json);
  std::unique_ptr<SerializerWorker> worker() override;

private:
  nlohmann::json& m_json;
};

}  // namespace omm::serialization
