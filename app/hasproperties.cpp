#include "hasproperties.h"


HasProperties::~HasProperties()
{
}

void HasProperties::add_property(const PropertyKey& key, std::unique_ptr<Property> property)
{
  assert(m_properties.count(key) == 0);
  m_properties[key] = std::move(property);
}

std::vector<HasProperties::PropertyKey> HasProperties::property_keys() const
{
  std::vector<PropertyKey> keys;
  keys.reserve(m_properties.size());
  for (auto&& item : m_properties) {
    keys.push_back(item.first);
  }
  return keys;
}