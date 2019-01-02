#pragma once

#include <glog/logging.h>
#include <cassert>
#include <map>
#include <memory>
#include <typeinfo>
#include <variant>

#include "properties/property.h"
#include "orderedmap.h"
#include "external/json_fwd.hpp"
#include "aspects/serializable.h"
#include "properties/typedproperty.h"
#include "properties/stringproperty.h"
#include "common.h"

namespace omm
{

class AbstractPropertyOwner : public virtual Serializable, public AbstractPropertyObserver
{
public:
  enum class Kind { None = 0x0,
                    Tag = 0x1, Style = 0x2, Object = 0x4, Tool = 0x8,
                    Item = Tag | Style | Object, All = Item | Tool };
  explicit AbstractPropertyOwner();
  explicit AbstractPropertyOwner(const AbstractPropertyOwner& other);
  AbstractPropertyOwner(AbstractPropertyOwner&& other);
  static constexpr auto TYPE = "PropertyOwner";
  virtual ~AbstractPropertyOwner();
  Property& property(const std::string& key) const;
  bool has_property(const std::string& key) const;
  template<typename ValueT> bool has_property(const std::string& key) const
  {
    if (has_property(key)) {
      const auto variant = property(key).variant_value();
      return std::get_if<ValueT>(&variant) != nullptr;
    } else {
      return false;
    }
  }

  const OrderedMap<std::string, Property>& properties() const;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual std::string name() const;
  void on_property_value_changed(Property& property) override;
  virtual Kind kind() const = 0;

  bool has_reference_cycle(const std::string& key) const;

  template<typename T> T* cast()
  {
    if (kind() == T::KIND) {
      return static_cast<T*>(this);
    } else {
      return nullptr;
    }
  }

  template<typename T, typename AbstractPropertyOwnerT, template<typename...> class ContainerT>
  static ContainerT<T*> cast(const ContainerT<AbstractPropertyOwnerT*>& ss)
  {
    const auto f = [](AbstractPropertyOwnerT* a) -> T* {
      return a->kind() == T::KIND ? static_cast<T*>(a) : nullptr;
    };
    return ::filter_if(::transform<T*>(ss, f), ::is_not_null);
  }

  static const std::string NAME_PROPERTY_KEY;

  template<typename PropertyT>
  PropertyT& add_property(const std::string& key, std::unique_ptr<PropertyT> property)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    PropertyT& ref = *property;
    assert(!m_properties.contains(key));
    m_properties.insert(key, std::move(property));
    ref.register_observer(*this);
    return ref;
  }

  template<typename PropertyT, typename... Args>
  PropertyT& add_property(const std::string& key, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    return add_property<PropertyT>(key, std::make_unique<PropertyT>(std::forward<Args>(args)...));
  }

  std::unique_ptr<Property> extract_property(const std::string& key);

private:
  OrderedMap<std::string, Property> m_properties;
};

template<AbstractPropertyOwner::Kind kind_> class PropertyOwner : public AbstractPropertyOwner
{
public:
  using AbstractPropertyOwner::AbstractPropertyOwner;
  static constexpr Kind KIND = kind_;
  Kind kind() const override { return KIND; }
};

}  // namespace omm

template<> struct EnableBitMaskOperators<omm::AbstractPropertyOwner::Kind> : std::true_type {};
