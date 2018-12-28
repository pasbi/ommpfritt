#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "properties/floatproperty.h"

namespace omm
{

PropertyItem::PropertyItem()
{
  set_property_type(FloatProperty::TYPE);
}

std::string PropertyItem::property_type() const
{
  return m_property->type();
}

QVariant PropertyItem::data(int role) const
{
  const QVariant data = QListWidgetItem::data(role);
  if (role == Qt::DisplayRole && data.isNull()) {
    return UNNAMED_PROPERTY_PLACEHOLDER;
  } else {
    return data;
  }
}

void PropertyItem::set_property_type(const std::string& type)
{
  const auto label = m_property ? m_property->label() : "";
  m_property = Property::make(type);
  m_property->set_label(label);
  m_property->set_category(Property::USER_PROPERTY_CATEGROY_NAME);
}

}  // namespace omm