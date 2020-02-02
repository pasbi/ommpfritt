#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <typeinfo>
#include <variant>
#include <QObject>

#include "orderedmap.h"
#include "external/json_fwd.hpp"
#include "aspects/serializable.h"
#include "common.h"
#include <Qt>
#include "aspects/typed.h"
#include "properties/property.h"

namespace omm
{

class Scene;
class Property;
class ReferenceProperty;

class AbstractPropertyOwner
    : public QObject
    , public virtual Serializable
    , public virtual Typed
{
  Q_OBJECT
public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PropertyOwner", "AbstractPropertyOwner");

protected:
  explicit AbstractPropertyOwner(Kind kind, Scene* scene);
  AbstractPropertyOwner(const AbstractPropertyOwner& other);
public:
  ~AbstractPropertyOwner() override;
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

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual QString name() const;

  static const QString NAME_PROPERTY_KEY;

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
  enum class CopiedProperties { Compatible = 0x1, New = 0x2, User = 0x3 };
  void copy_properties(AbstractPropertyOwner& target, CopiedProperties flags) const;

  std::size_t id() const;
  Scene* scene() const { return m_scene; }
  const Kind kind;

protected Q_SLOTS:
  virtual void on_property_value_changed(Property* property) { Q_UNUSED(property); }

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

};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::AbstractPropertyOwner::CopiedProperties> : std::true_type {};
