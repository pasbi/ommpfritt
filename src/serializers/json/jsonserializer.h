#pragma once

#include "external/json_fwd.hpp"
#include "serializers/abstractserializer.h"

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

}  // namespace omm
