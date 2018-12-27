#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "properties/floatproperty.h"

namespace omm
{

PropertyItem::PropertyItem()
{
  set_property_type<FloatProperty>();
}

std::string PropertyItem::property_type() const
{
  return m_property->type();
}

QVariant PropertyItem::data(int role) const
{
  if (role == Qt::DisplayRole) {
    return "Hello";
  } else {
    return QVariant();
  }
}

}  // namespace omm