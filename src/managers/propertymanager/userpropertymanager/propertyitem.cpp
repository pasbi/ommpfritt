#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "properties/floatproperty.h"

namespace omm
{

PropertyItem::PropertyItem(std::unique_ptr<Property> property)
  : m_property(std::move(property))
{
  if (!m_property) {
    set_property_type(FloatProperty::TYPE);
  } else {
    setText(QString::fromStdString(m_property->label()));
  }
}

std::string PropertyItem::property_type() const
{
  return m_property->type();
}

QVariant PropertyItem::data(int role) const
{
  const QVariant data = QListWidgetItem::data(role);
  if (role == Qt::DisplayRole && data.isNull()) {
    return QObject::tr(UNNAMED_PROPERTY_PLACEHOLDER);
  } else {
    return data;
  }
}

void PropertyItem::set_property_type(const std::string& type)
{
  const auto label = m_property ? m_property->label() : "";
  m_property = Property::make(type);
  m_property->set_label(label);
  const auto cat_name = QObject::tr(Property::USER_PROPERTY_CATEGROY_NAME.c_str());
  m_property->set_category(cat_name.toStdString());
}

}  // namespace omm