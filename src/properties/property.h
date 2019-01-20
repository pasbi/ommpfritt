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
class OptionsProperty;

class AbstractPropertyObserver
{
public:
  virtual void on_property_value_changed(Property& property) = 0;
};

class TriggerPropertyDummyValue
{
public:
  bool operator==(const TriggerPropertyDummyValue& other) const;
  bool operator!=(const TriggerPropertyDummyValue& other) const;
};

class Property
  : public AbstractFactory<std::string, Property>
  , public virtual Serializable
  , public Observed<AbstractPropertyObserver>
{
public:
  using variant_type = std::variant< bool, Color, double, int, AbstractPropertyOwner*,
                                     std::string, ObjectTransformation, size_t,
                                     TriggerPropertyDummyValue >;

  Property() = default;
  explicit Property(const Property& other) = default;
  virtual ~Property() = default;

  virtual variant_type variant_value() const = 0;
  virtual void notify_observers() = 0;

  virtual void set(const variant_type& value) = 0;

  template<typename EnumT> std::enable_if_t<std::is_enum_v<EnumT>, void>
  set(const EnumT& value) { set(static_cast<std::size_t>(value)); }

  template<typename ValueT> std::enable_if_t<!std::is_enum_v<ValueT>, ValueT>
  value() const { return std::get<ValueT>(variant_value()); }
  template<typename ValueT> std::enable_if_t<std::is_enum_v<ValueT>, ValueT>
  value() const { return static_cast<ValueT>(std::get<std::size_t>(variant_value())); }

  std::string label() const;
  std::string category() const;
  Property& set_label(const std::string& label);
  Property& set_category(const std::string& category);

  virtual std::string widget_type() const;

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

  virtual std::unique_ptr<Property> clone() const = 0;

  std::function<void(Property&)> pre_submit = [](Property& property) {};
  std::function<void(Property&)> post_submit = [](Property& property) {};
  bool wrap_with_macro = false;
  Property& set_pre_submit(const std::function<void(Property&)>& hook);
  Property& set_post_submit(const std::function<void(Property&)>& hook);

private:
  std::string m_label;
  std::string m_category;

public:
  OptionsProperty* enabled_buddy() const;
  bool is_enabled() const;
  template<typename EnumT> std::enable_if_t<std::is_enum_v<EnumT>, Property>&
  set_enabled_buddy(OptionsProperty& property, std::set<EnumT> values)
  {
    return set_enabled_buddy(property, ::transform<std::size_t>(values, [](EnumT value) {
      return static_cast<std::size_t>(value);
    }));
  }

private:
  Property& set_enabled_buddy(OptionsProperty& property, const std::set<std::size_t>& value);
  struct IsEnabledBuddy
  {
    OptionsProperty* property = nullptr;
    std::set<std::size_t> target_values;
  } m_enabled_buddy;

};

void register_properties();

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValue& v);
std::ostream& operator<<(std::ostream& ostream, const Property::variant_type& v);

}  // namespace ommAbstractPropertyOwner
