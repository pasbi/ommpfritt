#pragma once

#include "external/json.hpp"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"
#include "serializers/abstractdeserializer.h"
#include "serializers/abstractserializer.h"

namespace omm::serialization
{

class JSONSerializerWorker : public SerializerWorker
{
public:
  explicit JSONSerializerWorker(nlohmann::json& value);
  JSONSerializerWorker(const JSONSerializerWorker&) = delete;
  JSONSerializerWorker(JSONSerializerWorker&&) = delete;
  JSONSerializerWorker& operator=(const JSONSerializerWorker&) = delete;
  JSONSerializerWorker& operator=(JSONSerializerWorker&&) = delete;

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

class JSONDeserializer;

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

class JSONSerializer : public AbstractSerializer
{
public:
  explicit JSONSerializer(nlohmann::json& json);
  std::unique_ptr<SerializerWorker> sub(const std::string& key) override;
private:
  nlohmann::json& m_json;
};

class JSONDeserializer : public AbstractDeserializer
{
public:
  explicit JSONDeserializer(const nlohmann::json& json);
  std::unique_ptr<DeserializerWorker> sub(const std::string& key) override;
private:
  const nlohmann::json& m_json;
};

}  // namespace omm
