#pragma once

#include "external/json_fwd.hpp"
#include "serializers/abstractserializer.h"

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
