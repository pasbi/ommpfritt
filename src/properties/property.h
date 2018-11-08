#pragma once

#include <pybind11/embed.h>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include <glog/logging.h>

#include "objects/objecttransformation.h" //TODO remove that include
#include "external/json.hpp"
#include "observerregister.h"
#include "abstractfactory.h"
#include "serializers/serializable.h"

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

class Property : public AbstractFactory<std::string, Property>, public Serializable
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
  std::string label() const;
  std::string category() const;
  Property& set_label(const std::string& label);
  Property& set_category(const std::string& category);
  virtual std::string widget_type() const = 0;

  static void register_properties();

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;


private:
  std::string m_label;
  std::string m_category;
};

class AbstractTypedPropertyObserver
{
protected:
  virtual void on_value_changed() = 0;
  template<typename ValueT> friend class TypedProperty;
};


}  // namespace omm
