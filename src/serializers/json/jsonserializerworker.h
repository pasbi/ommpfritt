#pragma once

#include "external/json_fwd.hpp"
#include "serializers/serializerworker.h"

namespace omm::serialization
{

class JSONSerializerWorker : public SerializerWorker
{
public:
  explicit JSONSerializerWorker(nlohmann::json& value);

  void set_value(int value) override;
  void set_value(bool value) override;
  void set_value(double value) override;
  void set_value(const QString& value) override;
  void set_value(std::size_t id) override;
  void set_value(const Color& color) override;
  void set_value(const Vec2f& value) override;
  void set_value(const Vec2i& value) override;
  void set_value(const PolarCoordinates& value) override;
  void set_value(const TriggerPropertyDummyValueType&) override;
  void set_value(const SplineType&) override;

protected:
  std::unique_ptr<SerializationArray> start_array(std::size_t size) override;
  std::unique_ptr<SerializerWorker> sub(const std::string& key) override;
  std::unique_ptr<SerializerWorker> sub(const std::size_t i) override;

private:
  nlohmann::json& m_value;
};

}  // namespace omm::serialization
