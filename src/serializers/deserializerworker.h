#pragma once

#include "variant.h"
#include "serializers/array.h"
#include <QString>

namespace omm
{

class PolarCoordinates;

}  // namespace omm

namespace omm::serialization
{

template<typename T> concept Enum = std::is_enum_v<T>;
template<typename T> concept NotEnum = !std::is_enum_v<T>;

template<typename T>
concept Deserializable = requires(T s)
{
  { s.deserialize(std::declval<DeserializerWorker&>()) };
};

class AbstractDeserializer;

class DeserializerWorker
{
public:
  explicit DeserializerWorker(AbstractDeserializer& deserializer);
  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  AbstractDeserializer& deserializer() const;

  [[nodiscard]] virtual bool get_bool() = 0;
  [[nodiscard]] virtual int get_int() = 0;
  [[nodiscard]] virtual double get_double() = 0;
  [[nodiscard]] virtual QString get_string() = 0;
  [[nodiscard]] virtual std::size_t get_size_t() = 0;
  [[nodiscard]] virtual Color get_color() = 0;
  [[nodiscard]] virtual Vec2f get_vec2f() = 0;
  [[nodiscard]] virtual Vec2i get_vec2i() = 0;
  [[nodiscard]] virtual PolarCoordinates get_polarcoordinates() = 0;
  [[nodiscard]] virtual TriggerPropertyDummyValueType get_trigger_dummy_value() = 0;
  [[nodiscard]] virtual SplineType get_spline() = 0;
  virtual std::unique_ptr<DeserializerWorker> sub(const std::string& key) = 0;
  virtual std::unique_ptr<DeserializerWorker> sub(std::size_t i) = 0;
  virtual std::unique_ptr<DeserializationArray> start_array() = 0;
  virtual void end_array() {}

  template<typename A, typename B> void get(std::pair<A, B>& value)
  {
    sub("key")->get(value.first);
    sub("val")->get(value.second);
  }

  template<Deserializable T> void get(T& t)
  {
    t.deserialize(*this);
  }

  template<Enum T> void get(T& val)
  {
    val = static_cast<T>(get_size_t());
  }

  template<typename T> void get(T& val)
  {
    val = get<T>();
  }

  template<typename T> T get()
  {
    T t;
    get(t);
    return t;
  }

  template<typename T> void get(std::set<T>& value)
  {
    auto array = start_array();
    const auto n = array->size();
    value.clear();
    for (std::size_t i = 0; i < n; ++i) {
      value.insert(array->next().get<T>());
    }
  }

  template<typename T> void get(std::vector<T>& value)
  {
    auto array = start_array();
    const auto n = array->size();
    value.clear();
    value.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      value.empalce_back(array->next().get<T>());
    }
  }

  template<typename F> void get_items(const F& deserializer)
  {
    auto array = start_array();
    const auto n = array->size();
    for (std::size_t i = 0; i < n; ++i) {
      deserializer(array->next());
    }
  }

  variant_type get(const QString& type);

private:
  AbstractDeserializer& m_deserializer;

};

}  // namespace omm::serialization
