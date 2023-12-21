#pragma once

#include "serializers/abstractserializer.h"
#include <nlohmann/json_fwd.hpp>

namespace omm::serialization
{

class BinSerializer : public AbstractSerializer
{
public:
  explicit BinSerializer(QDataStream& stream);
  std::unique_ptr<SerializerWorker> worker() override;
private:
  QDataStream& m_stream;
};

}  // namespace omm
