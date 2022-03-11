#pragma once

#include "serializers/abstractdeserializer.h"
#include <QDataStream>

namespace omm::serialization
{

class BinDeserializer : public AbstractDeserializer
{
public:
  explicit BinDeserializer(QDataStream& stream);
  std::unique_ptr<DeserializerWorker> worker() override;
private:
  QDataStream& m_stream;
};

}  // namespace omm::serialization
