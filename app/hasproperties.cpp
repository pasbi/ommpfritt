#include "hasproperties.h"
#include "external/json.hpp"

omm::HasProperties::~HasProperties()
{
}

void omm::HasProperties::add_property( const omm::HasProperties::PropertyKey& key, 
                                       std::unique_ptr<omm::Property> property )
{
  assert(m_properties.count(key) == 0);
  m_properties[key] = std::move(property);
}

std::vector<omm::HasProperties::PropertyKey> omm::HasProperties::property_keys() const
{
  std::vector<PropertyKey> keys;
  keys.reserve(m_properties.size());
  for (auto&& item : m_properties) {
    keys.push_back(item.first);
  }
  return keys;
}

const omm::HasProperties::PropertyMap& omm::HasProperties::property_map() const
{
  return m_properties;
}


nlohmann::json omm::HasProperties::PropertyMap::to_json() const
{
  nlohmann::json o;
  for (const auto& item : *this) {
    const auto& key = item.first;
    const auto& property = item.second;
    o[key] = property->to_json();
  }
  return o;
}

