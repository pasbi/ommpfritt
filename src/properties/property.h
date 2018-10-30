#pragma once

#include <pybind11/embed.h>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include "objects/objecttransformation.h" //TODO remove that include
#include "external/json.hpp"

namespace py = pybind11;

namespace omm
{

template<typename ValueT> class TypedProperty;
class Object;

#define DISABLE_DANGEROUS_PROPERTY_TYPE(T) \
  static_assert(!std::is_same<ValueT, T>(), \
                "Forbidden Property Type '"#T"'.");

// Disable some T in TypedProperty<T>
// Without this guard, it was easy to accidentally use the 
// TypedProperty<const char*> acidentally when you actually want
// TypedProperty<std::string>. It's hard to notice and things will 
// break at strange places so debugging it becomes ugly.
#define DISABLE_DANGEROUS_PROPERTY_TYPES \
  DISABLE_DANGEROUS_PROPERTY_TYPE(const char*)


namespace {
// template<typename T>
// nlohmann::json to_json_value(const T& value)
// {
//   return value;
// }

// nlohmann::json to_json_value(const ObjectTransformation& t)
// {
//   return {};
// }

// nlohmann::json to_json_value(const Object*& t)
// {
//   return {};
// }

// nlohmann::json to_json_value(const std::string& t)
// {
//   return {};
// }


}  // namespace

class Property
{
public:
  Property();
  virtual ~Property();

  template<typename ValueT> bool is_type() const
  {
    return cast<ValueT>() != nullptr;
  }

  template<typename ValueT> const TypedProperty<ValueT>& cast() const
  {
    DISABLE_DANGEROUS_PROPERTY_TYPES
    return *dynamic_cast<const TypedProperty<ValueT>*>(this);
  }

  template<typename ValueT> TypedProperty<ValueT>& cast()
  {
    DISABLE_DANGEROUS_PROPERTY_TYPES
    return *dynamic_cast<TypedProperty<ValueT>*>(this);
  }

  virtual void set_py_object(const py::object& value) = 0;
  virtual py::object get_py_object() const = 0;
  virtual std::type_index type_index() const = 0;
  virtual nlohmann::json to_json() const = 0;
};

template<typename ValueT>
class TypedProperty : public Property
{
public:
  TypedProperty(ValueT defaultValue)
    : Property()
    , m_value(defaultValue)
    , m_defaultValue(defaultValue)
  {
  }

public:
  virtual void set_value(ValueT value)
  {
    m_value = value;
  }

  virtual ValueT value() const
  {
    return m_value;
  }

  virtual ValueT& value()
  {
    return m_value;
  }

  virtual void reset()
  {
    m_value = m_defaultValue;
  }

  py::object get_py_object() const override;

  void set_py_object(const py::object& value) override;
  // {
  //   return py::cast(m_value);
  // }
  // {
  //   m_value = value.cast<ValueT>();
  // }

  std::type_index type_index() const override
  {
    return std::type_index(typeid(ValueT));
  }

private:
  ValueT m_value;
  const ValueT m_defaultValue;
};

template<typename T>
class SimpleTypeProperty : public TypedProperty<T>
{
public:
  using TypedProperty<T>::TypedProperty;
  nlohmann::json to_json() const
  {
    return TypedProperty<T>::value();
  }
};

class IntegerProperty : public SimpleTypeProperty<int>
{
public:
  using SimpleTypeProperty<int>::SimpleTypeProperty;
};

class FloatProperty : public SimpleTypeProperty<double>
{
public:
  using SimpleTypeProperty<double>::SimpleTypeProperty;
};

class StringProperty : public SimpleTypeProperty<std::string>
{
public:
  using SimpleTypeProperty<std::string>::SimpleTypeProperty;
};

class TransformationProperty : public TypedProperty<ObjectTransformation>
{
public:
  using TypedProperty<ObjectTransformation>::TypedProperty;
  nlohmann::json to_json() const override;
};

class ReferenceProperty : public TypedProperty<Object*>
{
public:
  ReferenceProperty();
  static bool is_referenced(const Object* candidate);
  void set_value(Object* value) override;
  nlohmann::json to_json() const override;

private:
  static std::unordered_set<const Object*> m_references;
};

template<typename ValueT>
py::object TypedProperty<ValueT>::get_py_object() const
{
  return py::cast(m_value);
}

template<typename ValueT>
void TypedProperty<ValueT>::set_py_object(const py::object& value)
{
  m_value = value.cast<ValueT>();
}

}  // namespace omm
