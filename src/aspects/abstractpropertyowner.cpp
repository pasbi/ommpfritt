#include "aspects/abstractpropertyowner.h"

#include <QObject>

#include "animation/track.h"
#include "external/json.hpp"
#include "properties/referenceproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "serializers/abstractserializer.h"
#include "serializers/abstractdeserializer.h"
#include <random>

namespace
{
constexpr auto PROPERTIES_POINTER = "properties";
constexpr auto PROPERTY_TYPE_POINTER = "type";
constexpr auto PROPERTY_KEY_POINTER = "key";
constexpr auto ID_POINTER = "id";

}  // namespace

namespace omm
{
const QString AbstractPropertyOwner::NAME_PROPERTY_KEY = "name";

const OrderedMap<QString, Property>& AbstractPropertyOwner::properties() const
{
  return m_properties;
}

AbstractPropertyOwner::AbstractPropertyOwner(Kind kind, Scene* scene) : kind(kind), m_scene(scene)
{
}

AbstractPropertyOwner::AbstractPropertyOwner(const AbstractPropertyOwner& other)
    : QObject()  // NOLINT(readability-redundant-member-init)
      ,
      kind(other.kind), m_scene(other.m_scene)
{
  for (auto&& key : other.m_properties.keys()) {
    AbstractPropertyOwner::add_property(key, other.m_properties.at(key)->clone());
  }
}

AbstractPropertyOwner::~AbstractPropertyOwner()
{
  for (ReferenceProperty* ref_prop : m_referees) {
    QSignalBlocker blocker(ref_prop);
    ref_prop->set(nullptr);
  }
}

Property* AbstractPropertyOwner::property(const QString& key) const
{
  if (has_property(key)) {
    assert(m_properties.at(key) != nullptr);
    return m_properties.at(key).get();
  } else {
    return nullptr;
  }
}

bool AbstractPropertyOwner::has_property(const QString& key) const
{
  return m_properties.contains(key);
}

void AbstractPropertyOwner::serialize(serialization::SerializerWorker& worker) const
{
  worker.sub(ID_POINTER)->set_value(this);

  worker.sub(PROPERTIES_POINTER)->set_value(m_properties.keys(), [this](const auto& key, auto& worker_i) {
    const auto& property = *this->property(key);
    worker_i.sub(PROPERTY_KEY_POINTER)->set_value(key);
    worker_i.sub(PROPERTY_TYPE_POINTER)->set_value(property.type());
    property.serialize(worker_i);
  });
}

void AbstractPropertyOwner::deserialize(serialization::DeserializerWorker& worker)
{
  m_id = worker.sub(ID_POINTER)->get_size_t();
  worker.deserializer().register_reference(m_id, *this);

  worker.sub(PROPERTIES_POINTER)->get_items([this](auto& worker_i) {
    const auto property_key = worker_i.sub(PROPERTY_KEY_POINTER)->get_string();
    const auto property_type = worker_i.sub(PROPERTY_TYPE_POINTER)->get_string();

    if (properties().contains(property_key)) {
      assert(property_type == property(property_key)->type());
      property(property_key)->deserialize(worker_i);
    } else {
      std::unique_ptr<Property> property;
      try {
        property = Property::make(property_type);
      } catch (const std::out_of_range&) {
        const auto msg = "Failed to retrieve property type '" + property_type + "'.";
        throw serialization::AbstractDeserializer::DeserializeError(msg.toStdString());
      } catch (const Property::InvalidKeyError& e) {
        throw serialization::AbstractDeserializer::DeserializeError(e.what());
      }
      property->deserialize(worker_i);
      [[maybe_unused]] Property& ref = add_property(property_key, std::move(property));
      assert(ref.is_user_property());
    }
  });
}

Property& AbstractPropertyOwner::add_property(const QString& key,
                                              std::unique_ptr<Property> property)
{
  Property& ref = *property;
  assert(!m_properties.contains(key));
  assert(property.get() != nullptr);
  m_properties.insert(key, std::move(property));
  connect(&ref, &Property::value_changed, this, &AbstractPropertyOwner::on_property_value_changed);
  connect(&ref, &Property::value_changed, this, [this, key](Property* property) {
    assert(property != nullptr);
    if (Scene* scene = this->scene(); scene != nullptr) {
      Q_EMIT scene->mail_box().property_value_changed(*this, key, *property);
    }
  });
  return ref;
}

QString AbstractPropertyOwner::name() const
{
  return property(NAME_PROPERTY_KEY)->value<QString>();
}

std::unique_ptr<Property> AbstractPropertyOwner::extract_property(const QString& key)
{
  auto property = m_properties.extract(key);
  disconnect(property.get(), &Property::value_changed, this, nullptr);
  return property;
}

void AbstractPropertyOwner::copy_properties(AbstractPropertyOwner& target,
                                            CopiedProperties flags) const
{
  const auto keys = [](const AbstractPropertyOwner& o) {
    return util::transform<std::set>(o.properties().keys());
  };

  const auto target_keys = keys(target);
  const auto this_keys = keys(*this);
  if (!!(flags & CopiedProperties::Compatible)) {
    for (const auto& key : ::intersect(target_keys, this_keys)) {
      const auto& p = *property(key);
      auto& other_property = *target.property(key);
      if (other_property.is_compatible(p)) {
        other_property.set(p.variant_value());
      }
    }
  }

  for (const auto& key : this_keys) {
    const auto& p = *property(key);
    if (!target_keys.contains(key)) {
      const bool is_new = !!(flags & CopiedProperties::New);
      const bool is_user = !!(flags & CopiedProperties::User) && p.is_user_property();
      if (is_new || is_user) {
        target.add_property(key, p.clone());
      }
    }
  }
}

std::size_t AbstractPropertyOwner::id() const
{
  if (m_id == 0) {
    new_id();
  }
  return m_id;
}

void AbstractPropertyOwner::new_id() const
{
  static std::random_device rd;
  static std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with rd()
  static std::uniform_int_distribution<std::size_t> dis(1, std::numeric_limits<std::size_t>::max());
  m_id = dis(gen);
}

QString AbstractPropertyOwner::to_string() const
{
  return QString{"%1[]"}.arg(type());
}

bool AbstractPropertyOwner::pmatch(const Property* property, const std::set<QString>& keys) const
{
  return std::any_of(keys.begin(), keys.end(), [this, property](const QString& key) {
    return this->property(key) == property;
  });
}

}  // namespace omm
