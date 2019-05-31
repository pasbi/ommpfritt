#pragma once

#include <string>
#include <typeinfo>
#include <set>
#include <variant>
#include <functional>

#include "logging.h"
#include "external/json.hpp"
#include "observed.h"
#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "common.h"
#include "color/color.h"
#include "geometry/vec2.h"

namespace omm
{
class Object;

class Property;
class AbstractPropertyOwner;
class OptionsProperty;

class AbstractPropertyObserver
{
public:
  virtual ~AbstractPropertyObserver() = default;
  virtual void on_property_value_changed(Property& property, std::set<const void*> trace) = 0;
};

class TriggerPropertyDummyValueType
{
public:
  bool operator==(const TriggerPropertyDummyValueType& other) const;
  bool operator!=(const TriggerPropertyDummyValueType& other) const;
};

class Property
  : public AbstractFactory<std::string, Property>
  , public virtual Serializable
  , public Observed<AbstractPropertyObserver>
{
public:
  using variant_type = std::variant< bool, Color, double, int, AbstractPropertyOwner*,
                                     std::string, size_t, TriggerPropertyDummyValueType,
                                     Vec2f, Vec2i >;

  Property() = default;
  explicit Property(const Property& other) = default;
  virtual ~Property() = default;

  virtual variant_type variant_value() const = 0;
  void notify_observers(std::set<const void*> trace);

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
      LWARNING << "expected uniform value, but got " << values.size() << " different values.";
    }

    return *values.begin();
  }

  template<typename ResultT, typename MemFunc> static ResultT
  get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    return Property::get_value<ResultT, Property, MemFunc>(properties, std::forward<MemFunc>(f));
  }

  virtual bool is_compatible(const Property& other) const;
  static constexpr auto USER_PROPERTY_CATEGROY_NAME = QT_TRANSLATE_NOOP("Property",
                                                                        "user properties");

  // user properties can be added/edited/removed dynamically
  bool is_user_property() const;

  virtual std::unique_ptr<Property> clone() const = 0;

  std::function<void(Property&)> pre_submit = [](Property&) {};
  std::function<void(Property&)> post_submit = [](Property&) {};
  bool wrap_with_macro = false;
  Property& set_pre_submit(const std::function<void(Property&)>& hook);
  Property& set_post_submit(const std::function<void(Property&)>& hook);
  virtual void revise();

private:
  std::string m_label;
  std::string m_category;

public:
  Property* enabled_buddy() const;
  bool is_enabled() const;

  Property& set_enabled_buddy(Property& property, const std::function<bool(Property&)>& predicate);

  template<typename EnumT> std::enable_if_t<std::is_enum_v<EnumT>, Property&>&
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
    Property* property = nullptr;
    std::function<bool(Property&)> predicate;
    bool is_enabled() const;
  } m_enabled_buddy;

public:
  struct NotificationBlocker
  {
    explicit NotificationBlocker(Property& p);
    ~NotificationBlocker();
  private:
    Property& m_p;
  };
private:
  friend struct NotificationBlocker;
  bool m_notifications_are_blocked;
};

void register_properties();

std::ostream& operator<<(std::ostream& ostream, const TriggerPropertyDummyValueType& v);
std::ostream& operator<<(std::ostream& ostream, const Property::variant_type& v);

}  // namespace ommAbstractPropertyOwner
