#pragma once

#include <string>
#include <typeinfo>
#include <glog/logging.h>
#include <set>
#include <variant>
#include <functional>

#include "external/json.hpp"
#include "observed.h"
#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "common.h"
#include "color/color.h"
#include "geometry/objecttransformation.h"

namespace omm
{

#define DISABLE_DANGEROUS_PROPERTY_TYPE(T) \
  static_assert( !std::is_same<typename PropertyT::value_type, T>(), \
                 "Forbidden Property Type '"#T"'." );

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

class Property;
class AbstractPropertyOwner;

class AbstractPropertyObserver
{
public:
  virtual void on_property_value_changed(Property& property) = 0;
};

class Property
  : public AbstractFactory<std::string, Property>
  , public virtual Serializable
  , public Observed<AbstractPropertyObserver>
{
public:
  using variant_type = std::variant< bool, Color, double, int, AbstractPropertyOwner*,
                                     std::string, ObjectTransformation >;

  Property();
  virtual ~Property();

  virtual variant_type variant_value() const = 0;
  virtual void set(const variant_type& value) = 0;
  template<typename ValueT> ValueT value() const { return std::get<ValueT>(variant_value()); }


  std::string label() const;
  std::string category() const;
  Property& set_label(const std::string& label);
  Property& set_category(const std::string& category);
  virtual std::string widget_type() const = 0;

  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const;
  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root);

  template<typename ResultT, typename PropertyT, typename MemFunc> static
  ResultT get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    const auto values = ::transform<ResultT>(properties, [&f](const Property* property) {
      return f(static_cast<const PropertyT&>(*property));
    });

    if (values.size() > 1) {
      LOG(WARNING) << "expected uniform value, but got " << values.size() << " different values.";
    }

    return *values.begin();
  }

  template<typename ResultT, typename MemFunc> static ResultT
  get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    return Property::get_value<ResultT, Property, MemFunc>(properties, std::forward<MemFunc>(f));
  }

  virtual bool is_compatible(const Property& other) const;
  static const std::string USER_PROPERTY_CATEGROY_NAME;

  // user properties can be added/edited/removed dynamically
  bool is_user_property() const;

private:
  std::string m_label;
  std::string m_category;
};

void register_properties();

}  // namespace omm
