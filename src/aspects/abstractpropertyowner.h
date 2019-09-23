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
#include "aspects/typed.h"

namespace omm
{

class AbstractPropertyOwner
    : public QObject
    , public virtual Serializable
    , public virtual Typed
{
  Q_OBJECT
public:
  enum class Kind { None = 0x0,
                    Tag = 0x1, Style = 0x2, Object = 0x4, Tool = 0x8,
                    Item = Tag | Style | Object, All = Item | Tool };

  enum class Flag { None = 0x0,
                    Convertable = 0x1, HasScript = 0x2, IsPathLike = 0x4, IsView = 0x8 };

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PropertyOwner", "AbstractPropertyOwner");

protected:
  explicit AbstractPropertyOwner(Scene* scene);
  AbstractPropertyOwner(const AbstractPropertyOwner& other);
public:
  ~AbstractPropertyOwner() override;
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

  virtual Kind kind() const = 0;
  static const std::string NAME_PROPERTY_KEY;

  Property& add_property(const std::string& key, std::unique_ptr<Property> property);

  template<typename PropertyT, typename... Args>
  PropertyT& create_property(const std::string& key, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    auto property = std::make_unique<PropertyT>(std::forward<Args>(args)...);
    PropertyT& ref = *property;
    add_property(key, std::move(property));
    return ref;
  }

  std::unique_ptr<Property> extract_property(const std::string& key);
  void copy_properties(AbstractPropertyOwner& target) const;

  std::size_t id() const;
  Scene* scene() const { return m_scene; }

protected Q_SLOTS:
  virtual void on_property_value_changed(Property* property) { Q_UNUSED(property); }

private:
  OrderedMap<std::string, Property> m_properties;
  Scene* m_scene = nullptr;

  /**
   * @brief id_proposal if not 0 then this id shall be used for (de)serialization issues.
   *  if 0 then the proposal shall be ignored and a new id shall be generated instead.
   */
  mutable std::size_t m_id = 0;

public:
  // A set of ReferenceProperties which reference `this`.
  std::set<ReferenceProperty*> m_referees;

};


}  // namespace omm
