#pragma once

#include "serializers/deserializerworker.h"

namespace omm::serialization
{

class AbstractDeserializer;

class BinDeserializerWorker : public DeserializerWorker
{
public:
  explicit BinDeserializerWorker(AbstractDeserializer& deserializer, QDataStream& stream);

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  int get_int() override;
  double get_double() override;
  bool get_bool() override;
  QString get_string() override;
  std::size_t get_size_t() override;
  TriggerPropertyDummyValueType get_trigger_dummy_value() override;

protected:
  std::unique_ptr<DeserializationArray> start_array() override;
  std::unique_ptr<DeserializerWorker> sub(const std::string& key) override;
  std::unique_ptr<DeserializerWorker> sub(std::size_t i) override;

private:
  QDataStream& m_stream;
};


}  // namespace omm::serialization
