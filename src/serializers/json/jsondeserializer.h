#pragma once

#include "serializers/abstractdeserializer.h"
#include <nlohmann/json_fwd.hpp>

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
