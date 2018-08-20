#include "hasproperties.h"
#include "external/json.hpp"

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

const HasProperties::PropertyMap& HasProperties::property_map() const
{
  return m_properties;
}


nlohmann::json HasProperties::PropertyMap::to_json() const
{
  nlohmann::json o;
  for (const auto& item : *this) {
    const auto& key = item.first;
    const auto& property = item.second;
    o[key] = property->to_json();
  }
  return o;
}

