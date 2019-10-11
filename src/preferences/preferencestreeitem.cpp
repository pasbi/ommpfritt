#include "preferences/preferencestreeitem.h"

namespace omm
{

PreferencesTreeItem::PreferencesTreeItem(const std::string& name) : name(name) { }

PreferencesTreeValueItem::
PreferencesTreeValueItem(const std::string& group, const std::string& name, const std::string& value)
  : PreferencesTreeItem(name), group(group), m_value(value)
{
}

void PreferencesTreeValueItem::set_value(const std::string& value)
{
  if (value != m_value) {
    m_value = value;
    Q_EMIT value_changed(value);
  }
}

std::string PreferencesTreeValueItem::value() const { return m_value; }

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const std::string& group)
  : PreferencesTreeItem(group)
{
}



}  // namespace omm
