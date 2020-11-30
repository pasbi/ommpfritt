#include "properties/propertyconfiguration.h"

namespace omm
{
const PropertyConfiguration::value_type& PropertyConfiguration::get(const QString& key) const
{
  return m_store.at(key);
}

std::size_t PropertyConfiguration::count(const QString& key) const
{
  return m_store.count(key);
}

PropertyConfiguration::map_type::const_iterator
PropertyConfiguration::find(const QString& key) const
{
  return m_store.find(key);
}

PropertyConfiguration::map_type::const_iterator PropertyConfiguration::end() const
{
  return m_store.end();
}

}  // namespace omm
