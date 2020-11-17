#include "propertygroup.h"

namespace omm
{
PropertyGroup::PropertyGroup(const QString& prefix, AbstractPropertyOwner& property_owner)
    : m_prefix(prefix), m_property_owner(property_owner)
{
}

}  // namespace omm
