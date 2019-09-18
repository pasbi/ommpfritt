#pragma once

#include <iosfwd>
#include <unordered_map>

#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "color/color.h"
#include "geometry/vec2.h"
#include "geometry/polarcoordinates.h"
#include "variant.h"

namespace omm
{

class ObjectTransformation;
class Scene;
class ReferenceProperty;
class AbstractPropertyOwner;

class AbstractSerializer
  : public AbstractFactory<std::string, AbstractSerializer, std::ostream&>
{
public:
  using Pointer = Serializable::Pointer;
  explicit AbstractSerializer(std::ostream&) { }
  virtual ~AbstractSerializer() = default;

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual void start_array(size_t size, const Pointer& pointer) = 0;
  virtual void end_array() = 0;
  virtual void set_value(bool value, const Pointer& pointer) = 0;
  virtual void set_value(int value, const Pointer& pointer) = 0;
  virtual void set_value(double value, const Pointer& pointer) = 0;
  virtual void set_value(const std::string& value, const Pointer& pointer) = 0;
  virtual void set_value(const Vec2f& value, const Pointer& pointer) = 0;
  virtual void set_value(const Vec2i& value, const Pointer& pointer) = 0;
  virtual void set_value(const PolarCoordinates& value, const Pointer& pointer) = 0;
  virtual void set_value(const Color& color, const Pointer& pointer) = 0;
  virtual void set_value(const std::size_t, const Pointer& pointer) = 0;
  void set_value(const AbstractPropertyOwner* id, const Pointer& pointer);
  void set_value(const variant_type& variant, const Pointer& pointer);
  template<typename T> std::enable_if_t<std::is_enum_v<T>> set_value(const T& t, const Pointer& ptr)
  {
    set_value(static_cast<std::size_t>(t), ptr);
  }

  std::set<AbstractPropertyOwner*> serialized_references() const;

protected:
  void register_serialzied_reference(AbstractPropertyOwner* reference);
private:
  std::set<AbstractPropertyOwner*> m_serialized_references;

};

template<class T> struct always_false : std::false_type {};

class ReferencePolisher
{
protected:
  virtual void update_referenes(const std::map<std::size_t, AbstractPropertyOwner*>& map) = 0;
  friend class AbstractDeserializer;
};

class AbstractDeserializer
  : public AbstractFactory<std::string, AbstractDeserializer, std::istream&>
{
public:
  using Pointer = Serializable::Pointer;
  explicit AbstractDeserializer(std::istream&) { }
  virtual ~AbstractDeserializer();

  void add_references(const std::set<AbstractPropertyOwner*>& existing_references);

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual size_t array_size(const Pointer& pointer) = 0;
  virtual bool get_bool(const Pointer& pointer) = 0;
  virtual int  get_int(const Pointer& pointer) = 0;
  virtual double  get_double(const Pointer& pointer) = 0;
  virtual std::string  get_string(const Pointer& pointer) = 0;
  virtual std::size_t get_size_t(const Pointer& pointer) = 0;
  virtual Color get_color(const Pointer& pointer) = 0;
  virtual Vec2f get_vec2f(const Pointer& pointer) = 0;
  virtual Vec2i get_vec2i(const Pointer& pointer) = 0;
  virtual PolarCoordinates get_polarcoordinates(const Pointer& pointer) = 0;

  void register_reference(const std::size_t id, AbstractPropertyOwner& reference);
  void register_reference_polisher(ReferencePolisher& polisher);

  template<typename T> std::enable_if_t<!std::is_enum_v<T>, T> get(const Pointer&);
  template<typename T> std::enable_if_t<std::is_enum_v<T>, T> get(const Pointer& pointer)
  {
    return static_cast<T>(get_size_t(pointer));
  }

  variant_type get(const Pointer& pointer, const std::string& type);

  class DeserializeError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

private:
  // maps old stored hash to new ref
  std::map<std::size_t, AbstractPropertyOwner*> m_id_to_reference;
  std::set<ReferencePolisher*> m_reference_polishers;
};

void register_serializers();
void register_deserializers();


}  // namespace omm

