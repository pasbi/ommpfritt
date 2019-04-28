#pragma once

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
#include <Qt>
#include "observed.h"

namespace omm
{

class AbstractPropertyOwner;
class AbstractPropertyOwnerObserver
{
public:
  virtual ~AbstractPropertyOwnerObserver() = default;

  /**
   * @see AbstractPropertyOwner::on_change;
   */
  virtual void on_change(AbstractPropertyOwner* apo, int what, Property* property) = 0;
};

class AbstractPropertyOwner : public virtual Serializable,
                              public AbstractPropertyObserver,
                              public Observed<AbstractPropertyOwnerObserver>
{
public:
  enum class Kind { None = 0x0,
                    Tag = 0x1, Style = 0x2, Object = 0x4, Tool = 0x8,
                    Item = Tag | Style | Object, All = Item | Tool };

  enum class Flag { None = 0x0,
                    Convertable = 0x1, HasScript = 0x2, IsPathLike = 0x4, IsView = 0x8 };

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PropertyOwner", "AbstractPropertyOwner");

  Property* property(const std::string& key) const;
  bool has_property(const std::string& key) const;
  template<typename ValueT> bool has_property(const std::string& key) const
  {
    if (has_property(key)) {
      const auto variant = property(key)->variant_value();
      return std::get_if<ValueT>(&variant) != nullptr;
    } else {
      return false;
    }
  }

  virtual Flag flags() const = 0;

  const OrderedMap<std::string, Property>& properties() const;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual std::string name() const;
  void on_property_value_changed(Property& property) override;

  /**
   * @brief on_change is called when the appearance of `this` changed.
   *  That is, if `this` is hierarchical (i.e., Object) on_change is also called recursively
   *  if one of its children changed.
   * @param subject the item that has changed. For non-hierarchical items, subject equals this.
   * @param what a code about what has changed. New codes may be defined in subclasses.
   * @param property the property which has changed. May be nullptr if the change was not induced
   *  by a property.
   */
  virtual void on_change(AbstractPropertyOwner* subject, int what, Property* property);
  static constexpr auto PROPERTY_CHANGED = 0; // A property changed

  virtual Kind kind() const = 0;

  bool has_reference_cycle(const std::string& key) const;

  static const std::string NAME_PROPERTY_KEY;

  template<typename PropertyT>
  PropertyT& add_property(const std::string& key, std::unique_ptr<PropertyT> property)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    PropertyT& ref = *property;
    assert(!m_properties.contains(key));
    m_properties.insert(key, std::move(property));
    ref.Observed<AbstractPropertyObserver>::register_observer(*this);
    return ref;
  }

  template<typename PropertyT, typename... Args>
  PropertyT& add_property(const std::string& key, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    return add_property<PropertyT>(key, std::make_unique<PropertyT>(std::forward<Args>(args)...));
  }

  std::unique_ptr<Property> extract_property(const std::string& key);
  void copy_properties(AbstractPropertyOwner& target) const;

  std::size_t id() const;

private:
  OrderedMap<std::string, Property> m_properties;

  /**
   * @brief id_proposal if not 0 then this id shall be used for (de)serialization issues.
   *  if 0 then the proposal shall be ignored and a new id shall be generated instead.
   */
  mutable std::size_t m_id = 0;
};

template<AbstractPropertyOwner::Kind kind_> class PropertyOwner : public AbstractPropertyOwner
{
public:
  using AbstractPropertyOwner::AbstractPropertyOwner;
  virtual ~PropertyOwner() = default;
  static constexpr Kind KIND = kind_;
  Kind kind() const override { return KIND; }
};

std::ostream& operator<<(std::ostream& ostream, const AbstractPropertyOwner* apo);

template<typename T, typename S> T kind_cast(S s)
{
  if (s != nullptr && s->kind() == std::remove_pointer_t<T>::KIND) {
    return static_cast<T>(s);
  } else {
    return nullptr;
  }
}

template<typename T, template<typename...> class ContainerT>
ContainerT<T*> kind_cast(const ContainerT<AbstractPropertyOwner*>& ss)
{
  return ::filter_if(::transform<T*>(ss, [](auto* s) { return kind_cast<T*>(s); }), ::is_not_null);
}

template<typename T, template<typename...> class ContainerT>
ContainerT<AbstractPropertyOwner*> down_cast(const ContainerT<T*>& ss)
{
  return ::transform<AbstractPropertyOwner*>(ss, ::identity);
}

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::AbstractPropertyOwner::Kind> : std::true_type {};
template<> struct omm::EnableBitMaskOperators<omm::AbstractPropertyOwner::Flag> : std::true_type {};
