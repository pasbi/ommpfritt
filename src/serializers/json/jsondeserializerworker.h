#pragma once

#include "external/json_fwd.hpp"
#include "serializers/deserializerworker.h"

namespace omm::serialization
{

class AbstractDeserializer;

class JSONDeserializerWorker : public DeserializerWorker
{
public:
  explicit JSONDeserializerWorker(AbstractDeserializer& deserializer, const nlohmann::json& value);

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  int get_int() override;
  double get_double() override;
  bool get_bool() override;
  QString get_string() override;
  Color get_color() override;
  std::size_t get_size_t() override;
  Vec2f get_vec2f() override;
  Vec2i get_vec2i() override;
  PolarCoordinates get_polarcoordinates() override;
  TriggerPropertyDummyValueType get_trigger_dummy_value() override;
  SplineType get_spline() override;

protected:
  std::unique_ptr<DeserializationArray> start_array() override;
  std::unique_ptr<DeserializerWorker> sub(const std::string& key) override;
  std::unique_ptr<DeserializerWorker> sub(const std::size_t i) override;

private:
  const nlohmann::json& m_value;
};


}  // namespace omm::serialization
