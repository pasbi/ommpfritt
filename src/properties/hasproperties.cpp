#include "hasproperties.h"
#include "external/json.hpp"

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
  return ref;
}

const PropertyMap& HasProperties::properties() const
{
  return m_properties;
}

Property& HasProperties::property(const Key& key) const
{
  return *m_properties.at(key);
}

}  // namespace omm
