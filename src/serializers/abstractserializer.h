#pragma once

#include <iosfwd>
#include "abstractfactory.h"
#include "properties/propertymap.h"
#include "serializers/serializable.h"

namespace omm
{

class ObjectTransformation;
class Object;
class Scene;

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
  virtual void set_value(int value, const Pointer& pointer) = 0;
  virtual void set_value(double value, const Pointer& pointer) = 0;
  virtual void set_value(const std::string& value, const Pointer& pointer) = 0;
  virtual void set_value(const ObjectTransformation& value, const Pointer& pointer) = 0;
  virtual void set_value(const Object* value, const Pointer& pointer) = 0;

  static void register_serializers();
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
  virtual int  get_int(const Pointer& pointer) = 0;
  virtual double  get_double(const Pointer& pointer) = 0;
  virtual std::string  get_string(const Pointer& pointer) = 0;
  virtual ObjectTransformation get_object_transformation(const Pointer& pointer) = 0;
  virtual Object* get_object_reference(const Pointer& pointer) = 0;

  static void register_deserializers();

  class DeserializeError : public std::runtime_error
  {

  };
};

}  // namespace omm

