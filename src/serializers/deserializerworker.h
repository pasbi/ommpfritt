#pragma once

#include "variant.h"
#include "serializers/array.h"
#include <QString>

namespace omm
{
struct PolarCoordinates;
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
  virtual ~DeserializerWorker() = default;
  DeserializerWorker(const DeserializerWorker&) = delete;
  DeserializerWorker(DeserializerWorker&&) = delete;
  DeserializerWorker& operator=(const DeserializerWorker&) = delete;
  DeserializerWorker& operator=(DeserializerWorker&&) = delete;

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  [[nodiscard]] AbstractDeserializer& deserializer() const;

  [[nodiscard]] virtual bool get_bool() = 0;
  [[nodiscard]] virtual int get_int() = 0;
  [[nodiscard]] virtual double get_double() = 0;
  [[nodiscard]] virtual QString get_string() = 0;
  [[nodiscard]] virtual std::size_t get_size_t() = 0;
  [[nodiscard]] virtual TriggerPropertyDummyValueType get_trigger_dummy_value() = 0;
  [[nodiscard]] virtual std::unique_ptr<DeserializerWorker> sub(const std::string& key) = 0;
  [[nodiscard]] virtual std::unique_ptr<DeserializerWorker> sub(std::size_t i) = 0;
  [[nodiscard]] virtual std::unique_ptr<DeserializationArray> start_array() = 0;
  virtual void end_array() {}

  // since there's no partial function template specialization, overloading plays an important
  // rule when implementing the type-specific getters.
  // Hence, we implement all the
  //  `void get<T>(T& t)`   (return via reference argument)
  // and then implement a generic interface to
  //  T get<T>();
  // If we specialized `T get<T>()` directly, we wouldn't be able to distinguish deserialization
  // of, e.g., `vector` and `set`.
  // So if you want to implement deserialization support for a new type, specialize
  // `void get<T>(T&)` and everything else will just work automatically.

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

  template<typename T> void get(T& val);

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
      value.emplace_back(array->next().get<T>());
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

  template<typename T> [[nodiscard]] T get()
  {
    T t;
    get(t);
    return t;
  }

  [[nodiscard]] variant_type get(const QString& type);

private:
  AbstractDeserializer& m_deserializer;

};

}  // namespace omm::serialization
