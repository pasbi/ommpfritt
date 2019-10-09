#include "settingstreeitem.h"

namespace omm
{

SettingsTreeItem::SettingsTreeItem(const std::string& name) : name(name) { }

SettingsTreeValueItem::
SettingsTreeValueItem(const std::string& group, const std::string& name, const std::string& value)
  : SettingsTreeItem(name), group(group), m_value(value)
{
}

void SettingsTreeValueItem::set_value(const std::string& value)
{
  if (value != m_value) {
    m_value = value;
    Q_EMIT value_changed(value);
  }
}

std::string SettingsTreeValueItem::value() const { return m_value; }

SettingsTreeGroupItem::SettingsTreeGroupItem(const std::string& group)
  : SettingsTreeItem(group)
{
}



}  // namespace omm
