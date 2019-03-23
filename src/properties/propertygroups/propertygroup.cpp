#include "propertygroup.h"

namespace omm
{

PropertyGroup::PropertyGroup(const std::string& prefix, AbstractPropertyOwner& property_owner)
  : m_prefix(prefix), m_property_owner(property_owner)
{}

std::string PropertyGroup::key(const std::string& key) const
{
  return m_prefix + key;
}

}  // namespace omm
