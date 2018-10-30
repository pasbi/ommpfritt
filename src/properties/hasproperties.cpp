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
  bool was_inserted = m_properties.insert(key, std::move(property));
  assert(was_inserted);
  (void) was_inserted;
}

const std::vector<HasProperties::Key>& HasProperties::property_keys() const
{
  return m_properties.keys();
}

Property& HasProperties::property(const Key& key) const
{
  return *m_properties.at(key);
}

nlohmann::json HasProperties::to_json() const
{
  nlohmann::json keys = m_properties.keys();
  nlohmann::json values;

  for (const auto& key : m_properties) {
    values[key] = m_properties.at(key)->to_json();
  }

  return {
    { "properties", values },
    { "property_order", keys }
  };
}

}  // namespace omm
