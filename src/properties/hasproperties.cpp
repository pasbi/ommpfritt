#include "hasproperties.h"
#include "external/json.hpp"
#include "serializers/abstractserializer.h"

namespace
{
  constexpr auto PROPERTIES_POINTER = "properties";
  constexpr auto PROPERTY_TYPE_POINTER = "type";
  constexpr auto PROPERTY_KEY_POINTER = "key";
  constexpr auto ID_POINTER = "id";
}  // namespace

namespace omm
{

HasProperties::~HasProperties()
{
}

Property& HasProperties::add_property(const Key& key, std::unique_ptr<Property> property)
{
  Property& ref = *property;
  bool was_inserted = m_properties.insert(key, std::move(property));
  assert(was_inserted);
  (void) was_inserted;

  ref.register_observer(*this);
  return ref;
}

const PropertyMap& HasProperties::properties() const
{
  return m_properties;
}

Property& HasProperties::property(const Key& key) const
{
  assert(has_property(key));
  return *m_properties.at(key);
}

bool HasProperties::has_property(const Key& key) const
{
  return m_properties.contains(key);
}

void HasProperties::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  const auto id_pointer = make_pointer(root, ID_POINTER);
  serializer.set_value(identify(), id_pointer);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  serializer.start_array(m_properties.size(), properties_pointer);
  for (size_t i = 0; i < m_properties.size(); ++i) {
    const auto property_key = m_properties.keys().at(i);
    const auto property_pointer = make_pointer(properties_pointer, i);
    const auto& property = this->property(property_key);
    serializer.set_value(property_key, make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    serializer.set_value(property.type(), make_pointer(property_pointer, PROPERTY_TYPE_POINTER));
    property.serialize(serializer, property_pointer);
  }
  serializer.end_array();
}

void HasProperties::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);

  const auto id_pointer = make_pointer(root, ID_POINTER);
  const auto id = deserializer.get_id(id_pointer);
  deserializer.register_reference(id, *this);

  const auto properties_pointer = make_pointer(root, PROPERTIES_POINTER);
  size_t n_properties = deserializer.array_size(properties_pointer);
  for (size_t i = 0; i < n_properties; ++i) {
    const auto property_pointer = make_pointer(properties_pointer, i);

    const auto property_key =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_KEY_POINTER));
    const auto property_type =
      deserializer.get_string(make_pointer(property_pointer, PROPERTY_TYPE_POINTER));

    if (properties().contains(property_key)) {
      assert(property_type == property(property_key).type());
    } else {
      add_property(property_key, Property::make(property_type));
    }

    property(property_key).deserialize(deserializer, property_pointer);
  }
}

void HasProperties::on_property_value_changed()
{
}

}  // namespace omm
