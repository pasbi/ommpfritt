#pragma once

#include <QObject>
#include <cassert>
#include <map>
#include <memory>
#include <typeinfo>
#include <variant>

#include "aspects/typed.h"
#include "common.h"
#include "external/json_fwd.hpp"
#include "orderedmap.h"
#include "properties/property.h"
#include <Qt>

namespace omm
{
class Scene;
class Property;
class ReferenceProperty;

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class AbstractPropertyOwner : public QObject, virtual public Typed
{
  Q_OBJECT
public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PropertyOwner", "AbstractPropertyOwner");

protected:
  explicit AbstractPropertyOwner(Kind kind, Scene* scene);
  AbstractPropertyOwner(const AbstractPropertyOwner& other);

public:
  ~AbstractPropertyOwner() override;
  AbstractPropertyOwner(AbstractPropertyOwner&&) = delete;
  AbstractPropertyOwner& operator=(const AbstractPropertyOwner&) = delete;
  AbstractPropertyOwner& operator=(AbstractPropertyOwner&&) = delete;

  [[nodiscard]] bool eq(const AbstractPropertyOwner& other, const auto& eq) const
  {
    const auto keys = m_properties.keys();
    if (keys != other.m_properties.keys()) {
      return false;
    }

    for (const auto& key : keys) {
      if (!eq(m_properties.at(key)->variant_value(), other.m_properties.at(key)->variant_value())) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool eq(const AbstractPropertyOwner& other) const
  {
    return eq(other, std::equal_to<>{});
  }

  Property* property(const QString& key) const;
  bool has_property(const QString& key) const;
  template<typename ValueT> bool has_property(const QString& key) const
  {
    if (has_property(key)) {
      const auto variant = property(key)->variant_value();
      return std::get_if<ValueT>(&variant) != nullptr;
    } else {
      return false;
    }
  }

  virtual Flag flags() const = 0;

  const OrderedMap<QString, Property>& properties() const;

  virtual void serialize(serialization::SerializerWorker& worker) const;
  virtual void deserialize(serialization::DeserializerWorker& worker);
  virtual QString name() const;

  static const QString NAME_PROPERTY_KEY;

  /**
   * @brief add_property adds a property to this abstract PropertyOwner.
   * @param key the key
   * @param property the property
   * @return a reference to the added property.
   * @note this function is used in the copy constructor, although it's virtual.
   *  Since one cannot call virtual members in constructors, it is called statically.
   *  That means, if your class relies on some overriden `add_property`-method, it should
   *  delete the copy-constructor since it probably will not do what you expect.
   *  An example is the Node class: it overrides `add_property`, consequently deletes the
   *  copy constructor and implements a serialization-based copy mechanism instead.
   */
  virtual Property& add_property(const QString& key, std::unique_ptr<Property> property);

  template<typename PropertyT, typename... Args>
  PropertyT& create_property(const QString& key, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    auto property = std::make_unique<PropertyT>(std::forward<Args>(args)...);
    PropertyT& ref = *property;
    add_property(key, std::move(property));
    return ref;
  }

  virtual std::unique_ptr<Property> extract_property(const QString& key);
  enum class CopiedProperties { Compatible = 0x1, New = 0x2, User = 0x4 };
  void copy_properties(AbstractPropertyOwner& target, CopiedProperties flags) const;

  std::size_t id() const;
  Scene* scene() const
  {
    return m_scene;
  }
  const Kind kind;
  void new_id() const;

  virtual QString to_string() const;

protected:
  virtual void on_property_value_changed(omm::Property* property)
  {
    Q_UNUSED(property);
  }

Q_SIGNALS:
  void property_visibility_changed();

private:
  OrderedMap<QString, Property> m_properties;
  Scene* m_scene = nullptr;

  /**
   * @brief id_proposal if not 0 then this id shall be used for (de)serialization issues.
   *  if 0 then the proposal shall be ignored and a new id shall be generated instead.
   */
  mutable std::size_t m_id = 0;

public:
  // A set of ReferenceProperties which reference `this`.
  std::set<ReferenceProperty*> m_referees;

protected:
  bool pmatch(const Property* property, const std::set<QString>& keys) const;
};

}  // namespace omm

template<>
struct omm::EnableBitMaskOperators<omm::AbstractPropertyOwner::CopiedProperties> : std::true_type {
};
