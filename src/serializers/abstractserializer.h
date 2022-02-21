#pragma once

#include <QObject>
#include <iosfwd>
#include <unordered_map>

#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "common.h"
#include "geometry/vec2.h"
#include "variant.h"

namespace omm
{

class AbstractPropertyOwner;
class Color;
class ObjectTransformation;
class ReferenceProperty;
class Scene;
struct PolarCoordinates;

template<typename T>
concept SizeIterable = requires(const T& a)
{
  { a.size() } -> std::convertible_to<std::size_t>;
  { a.begin() } -> std::convertible_to<typename T::const_iterator>;
  { a.end() } -> std::convertible_to<typename T::const_iterator>;
};

class AbstractSerializer : public QObject
{
  Q_OBJECT
private:
  template<typename T, typename = void> struct is_iterable : std::false_type {
  };
  template<typename T>
  struct is_iterable<T,
                     std::void_t<decltype(std::declval<T>().size()),
                                 decltype(std::declval<T>().begin()),
                                 decltype(std::declval<T>().end())>> : std::true_type {
  };
  template<typename T> static inline constexpr bool is_iterable_v = is_iterable<T>::value;

public:
  using Pointer = Serializable::Pointer;
  explicit AbstractSerializer(std::ostream&)
  {
  }

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual void set_value(bool value, const Pointer& pointer) = 0;
  virtual void set_value(int value, const Pointer& pointer) = 0;
  virtual void set_value(double value, const Pointer& pointer) = 0;
  virtual void set_value(const QString& value, const Pointer& pointer) = 0;
  virtual void set_value(const Vec2f& value, const Pointer& pointer) = 0;
  virtual void set_value(const Vec2i& value, const Pointer& pointer) = 0;
  virtual void set_value(const PolarCoordinates& value, const Pointer& pointer) = 0;
  virtual void set_value(const Color& color, const Pointer& pointer) = 0;
  virtual void set_value(std::size_t, const Pointer& pointer) = 0;
  virtual void set_value(const TriggerPropertyDummyValueType&, const Pointer& pointer) = 0;
  virtual void set_value(const SplineType&, const Pointer& pointer) = 0;
  void set_value(const AbstractPropertyOwner* ref, const Pointer& pointer);
  void set_value(const variant_type& variant, const Pointer& pointer);
  void set_value(const Serializable& serializable, const Pointer& pointer);
  template<typename T> std::enable_if_t<std::is_enum_v<T>> set_value(const T& t, const Pointer& ptr)
  {
    set_value(static_cast<std::size_t>(t), ptr);
  }

  template<typename A, typename B> void set_value(const std::pair<A, B>& pair, const Pointer& ptr)
  {
    set_value(pair.first, Serializable::make_pointer(ptr, "key"));
    set_value(pair.second, Serializable::make_pointer(ptr, "val"));
  }

  template<SizeIterable Vs, typename F>
  void set_value(const Vs& vs, const Pointer& pointer, const F& serializer)
      requires requires(const F& serializer) { serializer(*vs.begin(), Pointer{}); }
  {
    start_array(vs.size(), pointer);
    std::size_t i = 0;
    for (const auto& v : vs) {
      serializer(v, Serializable::make_pointer(pointer, i));
      i += 1;
    }
    end_array();
  }

  template<SizeIterable Vs> void set_value(const Vs& vs, const Pointer& ptr)
  {
    set_value(vs, ptr, [this](const auto& v, const auto& root) {
      this->set_value(v, root);
    });
  }

protected:
  void register_serialzied_reference(AbstractPropertyOwner* reference);
  virtual void start_array(std::size_t size, const Pointer& pointer) = 0;
  virtual void end_array() = 0;
};

class ReferencePolisher
{
public:
  virtual ~ReferencePolisher() = default;
  ReferencePolisher() = default;
  ReferencePolisher(ReferencePolisher&&) = default;
  ReferencePolisher(const ReferencePolisher&) = default;
  ReferencePolisher& operator=(ReferencePolisher&&) = default;
  ReferencePolisher& operator=(const ReferencePolisher&) = default;

protected:
  virtual void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) = 0;
  friend class AbstractDeserializer;
};

class AbstractDeserializer : public QObject
{
  Q_OBJECT
public:
  using Pointer = Serializable::Pointer;
  explicit AbstractDeserializer(std::istream&)
  {
  }

  /**
   * @brief polish sets all registered references.
   */
  void polish();

  void add_references(const std::set<AbstractPropertyOwner*>& existing_references);

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  [[nodiscard]] virtual bool get_bool(const Pointer& pointer) = 0;
  [[nodiscard]] virtual int get_int(const Pointer& pointer) = 0;
  [[nodiscard]] virtual double get_double(const Pointer& pointer) = 0;
  [[nodiscard]] virtual QString get_string(const Pointer& pointer) = 0;
  [[nodiscard]] virtual std::size_t get_size_t(const Pointer& pointer) = 0;
  [[nodiscard]] virtual Color get_color(const Pointer& pointer) = 0;
  [[nodiscard]] virtual Vec2f get_vec2f(const Pointer& pointer) = 0;
  [[nodiscard]] virtual Vec2i get_vec2i(const Pointer& pointer) = 0;
  [[nodiscard]] virtual PolarCoordinates get_polarcoordinates(const Pointer& pointer) = 0;
  [[nodiscard]] virtual TriggerPropertyDummyValueType get_trigger_dummy_value(const Pointer& pointer) = 0;
  [[nodiscard]] virtual SplineType get_spline(const Pointer& pointer) = 0;

  void register_reference(std::size_t id, AbstractPropertyOwner& reference);
  void register_reference_polisher(std::unique_ptr<ReferencePolisher> polisher);

  template<typename T> std::enable_if_t<!std::is_enum_v<T>, T> get(const Pointer&);
  template<typename T> std::enable_if_t<std::is_enum_v<T>, T> get(const Pointer& pointer)
  {
    return static_cast<T>(get_size_t(pointer));
  }

  template<typename T> void get(T& value, const Pointer& pointer)
  {
    if constexpr (std::is_base_of_v<Serializable, T>) {
      value.deserialize(*this, pointer);
    } else {
      value = get<T>(pointer);
    }
  }

  template<typename A, typename B> void get(std::pair<A, B>& value, const Pointer& pointer)
  {
    get(value.first, Serializable::make_pointer(pointer, "key"));
    get(value.second, Serializable::make_pointer(pointer, "val"));
  }

  template<typename T> void get(std::set<T>& value, const Pointer& pointer)
  {
    const std::size_t n = array_size(pointer);
    for (std::size_t i = 0; i < n; ++i) {
      T v;
      get(v, Serializable::make_pointer(pointer, i));
      value.insert(std::move(v));
    }
  }

  template<typename T> void get(std::vector<T>& value, const Pointer& pointer)
  {
    const std::size_t n = array_size(pointer);
    value.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      T v;
      get(v, Serializable::make_pointer(pointer, i));
      value.push_back(std::move(v));
    }
  }

  template<typename F> void get_items(const Pointer& root, const F& deserializer)
  {
    const auto array_size = this->array_size(root);
    for (std::size_t i = 0; i < array_size; ++i) {
      deserializer(Serializable::make_pointer(root, i));
    }
  }

  void get(Serializable& serializable, const Pointer& pointer);

  variant_type get(const Pointer& pointer, const QString& type);

  class DeserializeError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

protected:
  [[nodiscard]] virtual std::size_t array_size(const Pointer& pointer) = 0;

private:
  // maps old stored hash to new ref
  std::map<std::size_t, AbstractPropertyOwner*> m_id_to_reference;
  std::set<std::unique_ptr<ReferencePolisher>> m_reference_polishers;
};

}  // namespace omm
