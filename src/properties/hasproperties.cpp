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
  m_property_keys.push_back(key);
}

std::vector<HasProperties::Key> HasProperties::property_keys() const
{
  std::vector<Key> keys;
  keys.reserve(m_properties.size());
  std::transform( m_properties.begin(), m_properties.end(), std::back_inserter(keys),
                  [](const auto& value) { return value.first; } );
  return keys;
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

Property& HasProperties::property(const Key& key) const
{
  return *m_properties.at(key);
}

nlohmann::json HasProperties::to_json() const
{
  return {
    { "properties:", m_properties.to_json() }
  };
}

}  // namespace omm
