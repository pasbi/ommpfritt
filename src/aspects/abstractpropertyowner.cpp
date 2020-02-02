#include "aspects/abstractpropertyowner.h"

#include <QObject>

#include "external/json.hpp"
#include "serializers/abstractserializer.h"
#include "properties/referenceproperty.h"
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "animation/track.h"

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

AbstractPropertyOwner::AbstractPropertyOwner(Kind kind, Scene *scene)
  : kind(kind), m_scene(scene)
{
}

AbstractPropertyOwner::AbstractPropertyOwner(const AbstractPropertyOwner &other)
  : kind(other.kind), m_scene(other.m_scene)
{
  for (auto&& key : other.m_properties.keys()) {
    add_property(key, other.m_properties.at(key)->clone());
  }
}

AbstractPropertyOwner::~AbstractPropertyOwner()
{
  for (ReferenceProperty* ref_prop : m_referees) {
    QSignalBlocker blocker(ref_prop);
    ref_prop->set(nullptr);
  }
}

Property *AbstractPropertyOwner::property(const QString& key) const
{
  if (has_property(key)) {
    return m_properties.at(key).get();
  } else {
    return nullptr;
  }
}

bool AbstractPropertyOwner::has_property(const QString& key) const
{
  return m_properties.contains(key);
}

void AbstractPropertyOwner::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  const auto id_pointer = make_pointer(root, ID_POINTER);
  serializer.set_value(this, id_pointer);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  serializer.start_array(m_properties.size(), properties_pointer);
  for (size_t i = 0; i < m_properties.size(); ++i) {
    const auto property_key = m_properties.keys().at(i);
    const auto property_pointer = make_pointer(properties_pointer, i);
    const auto& property = *this->property(property_key);
    serializer.set_value(property_key, make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    serializer.set_value(property.type(), make_pointer(property_pointer, PROPERTY_TYPE_POINTER));
    property.serialize(serializer, property_pointer);
  }
  serializer.end_array();
}

void AbstractPropertyOwner::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  const auto id_pointer = make_pointer(root, ID_POINTER);
  m_id = deserializer.get_size_t(id_pointer);
  deserializer.register_reference(m_id, *this);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  size_t n_properties = deserializer.array_size(properties_pointer);
  for (size_t i = 0; i < n_properties; ++i) {
    const auto property_pointer = make_pointer(properties_pointer, i);

    const auto property_key =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    const auto property_type =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_TYPE_POINTER));

    if (properties().contains(property_key)) {
      assert(property_type == property(property_key)->type());
      property(property_key)->deserialize(deserializer, property_pointer);
    } else {
      std::unique_ptr<Property> property;
      try {
        property = Property::make(property_type);
      } catch (const std::out_of_range&) {
        const auto msg = "Failed to retrieve property type '" + property_type + "'.";
        throw AbstractDeserializer::DeserializeError(msg.toStdString());
      }
      property->deserialize(deserializer, property_pointer);
      add_property(property_key, std::move(property));
    }

  }
}

Property
&AbstractPropertyOwner::add_property(const QString &key, std::unique_ptr<Property> property)
{
  Property& ref = *property;
  assert(!m_properties.contains(key));
  m_properties.insert(key, std::move(property));
  connect(&ref, SIGNAL(value_changed(Property*)),
          this, SLOT(on_property_value_changed(Property*)));
  connect(&ref, &Property::value_changed, [this, key](Property* property) {
    assert(property != nullptr);
    if (Scene* scene = this->scene(); scene != nullptr) {
      Q_EMIT scene->message_box().property_value_changed(*this, key, *property);
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

std::ostream& operator<<(std::ostream& ostream, const AbstractPropertyOwner* apo)
{
  if (apo == nullptr) {
    ostream << "AbstractPropertyOwner[nullptr]";
  } else {
    const auto kind = static_cast<std::underlying_type_t<decltype(apo->kind)>>(apo->kind);
    ostream << "AbstractPropertyOwner[" << kind << "]";
  }
  return ostream;
}

void AbstractPropertyOwner::copy_properties(AbstractPropertyOwner& target, CopiedProperties flags) const
{
  const auto keys = [](const AbstractPropertyOwner& o) {
    return ::transform<QString, std::set>(o.properties().keys(), ::identity);
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
    const bool key_exists = ::contains(target_keys, key);
    if (!!(flags & CopiedProperties::New) && !key_exists) {
      target.add_property(key, p.clone());
    } else if (!!(flags & CopiedProperties::User) && p.is_user_property() && !key_exists) {
      target.add_property(key, p.clone());
    }
  }
}

std::size_t AbstractPropertyOwner::id() const
{
  if (m_id == 0) {
    m_id = std::hash<const void*>()(this);
  }
  return m_id;
}

}  // namespace omm
