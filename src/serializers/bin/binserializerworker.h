#pragma once

#include "external/json_fwd.hpp"
#include "serializers/serializerworker.h"

namespace omm::serialization
{

class BinSerializerWorker : public SerializerWorker
{
public:
  explicit BinSerializerWorker(QDataStream& stream);

  void set_value(int value) override;
  void set_value(bool value) override;
  void set_value(double value) override;
  void set_value(const QString& value) override;
  void set_value(std::size_t value) override;
  void set_value(const TriggerPropertyDummyValueType&) override;

protected:
  std::unique_ptr<SerializationArray> start_array(std::size_t size) override;
  std::unique_ptr<SerializerWorker> sub(const std::string& key) override;
  std::unique_ptr<SerializerWorker> sub(std::size_t i) override;

private:
  QDataStream& m_stream;
};

}  // namespace omm::serialization
