#pragma once

#include "external/json_fwd.hpp"
#include "serializers/abstractdeserializer.h"

namespace omm::serialization
{

class JSONDeserializer : public AbstractDeserializer
{
public:
  explicit JSONDeserializer(const nlohmann::json& json);
  std::unique_ptr<DeserializerWorker> worker() override;
private:
  const nlohmann::json& m_json;
};

}  // namespace omm::serialization
