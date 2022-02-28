#pragma once

#include "variant.h"
#include <QString>
#include "serializers/array.h"

namespace omm
{

class PolarCoordinates;
class Serializable;

}  // namespace omm

namespace omm::serialization
{

template<typename T>
concept SizeIterable = requires(const T& a)
{
  { a.size() } -> std::convertible_to<std::size_t>;
  { a.begin() } -> std::convertible_to<typename T::const_iterator>;
  { a.end() } -> std::convertible_to<typename T::const_iterator>;
};

template<typename T>
concept SerializableConcept = requires(const T& s)
{
  { s.serialize(std::declval<SerializerWorker&>()) };
};

class SerializerWorker
{
public:
  explicit SerializerWorker() = default;
  virtual ~SerializerWorker() = default;
  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual void set_value(bool value) = 0;
  virtual void set_value(int value) = 0;
  virtual void set_value(double value) = 0;
  virtual void set_value(const QString& value) = 0;
  virtual void set_value(const Vec2f& value) = 0;
  virtual void set_value(const Vec2i& value) = 0;
  virtual void set_value(const PolarCoordinates& value) = 0;
  virtual void set_value(const Color& color) = 0;
  virtual void set_value(std::size_t) = 0;
  virtual void set_value(const TriggerPropertyDummyValueType&) = 0;
  virtual void set_value(const SplineType&) = 0;
  void set_value(const AbstractPropertyOwner* ref);
  void set_value(const variant_type& variant);
  void set_value(const Serializable& serializable);
  template<typename T> std::enable_if_t<std::is_enum_v<T>> set_value(const T& t)
  {
    set_value(static_cast<std::size_t>(t));
  }

  template<SerializableConcept T> void set_value(const T& t)
  {
    t.serialize(*this);
  }

  template<typename A, typename B> void set_value(const std::pair<A, B>& pair)
  {
    sub("key")->set_value(pair.first);
    sub("val")->set_value(pair.second);
  }

  template<SizeIterable Vs, typename F>
  void set_value(const Vs& vs, const F& serializer)
      requires requires(const F& serializer) { serializer(*vs.begin(), *this); }
  {
    auto array = start_array(vs.size());
    for (const auto& v : vs) {
      serializer(v, array->next());
    }
  }

  template<SizeIterable Vs> void set_value(const Vs& vs)
  {
    auto array = start_array(vs.size());
    for (const auto& v : vs) {
      array->next().set_value(v);
    }
  }

  virtual std::unique_ptr<SerializerWorker> sub(const std::string& key) = 0;
  virtual std::unique_ptr<SerializerWorker> sub(std::size_t i) = 0;
  virtual void end_array() {}

protected:
  virtual std::unique_ptr<SerializationArray> start_array(std::size_t size) = 0;
};

}  // namespace omm::serialization