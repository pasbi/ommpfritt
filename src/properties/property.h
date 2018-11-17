#pragma once

#include <pybind11/embed.h>
#include <string>
#include <typeinfo>
#include <glog/logging.h>
#include <set>

#include "external/json.hpp"
#include "observed.h"
#include "abstractfactory.h"
#include "serializers/serializable.h"
#include "common.h"

namespace py = pybind11;

namespace omm
{

#define DISABLE_DANGEROUS_PROPERTY_TYPE(T) \
  static_assert(!std::is_same<ValueT, T>(), "Forbidden Property Type '"#T"'.");

// Disable some T in TypedProperty<T>
// Without this guard, it was easy to accidentally use e.g.
// TypedProperty<const char*> when you actually want
// TypedProperty<std::string>. It's hard to notice, things will
// break at strange places and debugging it becomes ugly.
// If you encounter such an evil twin ET for your type T, add it to the following list like I did
// for `const char*`.
#define DISABLE_DANGEROUS_PROPERTY_TYPES \
  DISABLE_DANGEROUS_PROPERTY_TYPE(const char*)

template<typename ValueT> class TypedProperty;
class Object;

class AbstractPropertyObserver
{
public:
  virtual void on_property_value_changed() = 0;
};

class Property
  : public AbstractFactory<std::string, Property>
  , public Serializable
  , public Observed<AbstractPropertyObserver>
{
public:
  using SetOfProperties = std::set<Property*>;

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
  std::string label() const;
  std::string category() const;
  Property& set_label(const std::string& label);
  Property& set_category(const std::string& category);
  virtual std::string widget_type() const = 0;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  static std::string get_label(const SetOfProperties& properties);

  template<typename ValueT> static auto cast_all(const SetOfProperties& properties)
  {
    return ::transform<TypedProperty<ValueT>*>(properties, [](Property* property) {
      return &property->cast<ValueT>();
    });
  }

  template<typename ValueT>
  static SetOfProperties cast_all(const std::set<TypedProperty<ValueT>*>& properties)
  {
    return ::transform<Property*>(properties, [](auto* property) {
      return static_cast<Property*>(property);
    });
  }

private:
  std::string m_label;
  std::string m_category;
};

void register_properties();

}  // namespace omm
