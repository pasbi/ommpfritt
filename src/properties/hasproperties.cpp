#include "hasproperties.h"
#include "external/json.hpp"

namespace omm
{

HasProperties::~HasProperties()
{
}

void HasProperties::add_property( const HasProperties::Key& key,
                                       std::unique_ptr<Property> property )
{
  assert(m_properties.count(key) == 0);
  m_properties[key] = std::move(property);
}

std::unordered_set<HasProperties::Key> HasProperties::property_keys() const
{
  std::unordered_set<Key> keys;
  keys.reserve(m_properties.size());
  std::transform( m_properties.begin(), m_properties.end(), std::inserter(keys, keys.begin()),
                  [](const auto& value) { return value.first; } );
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

}  // namespace omm
