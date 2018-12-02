#pragma once

#include <iosfwd>
#include <unordered_map>

#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "color/color.h"

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
  explicit AbstractSerializer(std::ostream& stream);
  virtual ~AbstractSerializer();

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual void start_array(size_t size, const Pointer& pointer) = 0;
  virtual void end_array() = 0;
  virtual void set_value(bool value, const Pointer& pointer) = 0;
  virtual void set_value(int value, const Pointer& pointer) = 0;
  virtual void set_value(double value, const Pointer& pointer) = 0;
  virtual void set_value(const std::string& value, const Pointer& pointer) = 0;
  virtual void set_value(const ObjectTransformation& value, const Pointer& pointer) = 0;
  virtual void set_value(const Serializable::IdType& id, const Pointer& pointer) = 0;
  virtual void set_value(const Color& color, const Pointer& pointer) = 0;


};

class AbstractDeserializer
  : public AbstractFactory<std::string, AbstractDeserializer, std::istream&>
{
public:
  using Pointer = Serializable::Pointer;
  explicit AbstractDeserializer(std::istream& stream);
  virtual ~AbstractDeserializer();

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  virtual size_t array_size(const Pointer& pointer) = 0;
  virtual bool get_bool(const Pointer& pointer) = 0;
  virtual int  get_int(const Pointer& pointer) = 0;
  virtual double  get_double(const Pointer& pointer) = 0;
  virtual std::string  get_string(const Pointer& pointer) = 0;
  virtual ObjectTransformation get_object_transformation(const Pointer& pointer) = 0;
  virtual Serializable::IdType get_id(const Pointer& pointer) = 0;
  virtual Color get_color(const Pointer& pointer) = 0;


  void register_reference(const Serializable::IdType& id, AbstractPropertyOwner& reference);
  void register_reference_property( ReferenceProperty& reference_property,
                                    const Serializable::IdType& id );


  class DeserializeError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

private:
  // maps old stored hash to new ref
  std::unordered_map<Serializable::IdType, AbstractPropertyOwner*> m_id_to_reference;

  // maps new property to old hash
  std::unordered_map<ReferenceProperty*, Serializable::IdType> m_reference_property_to_id;
};

void register_serializers();
void register_deserializers();

}  // namespace omm

