#include "preferences/preferencestreeitem.h"
#include "logging.h"

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

void PreferencesTreeValueItem::set_value(std::size_t column, const std::string& value)
{
  QStringList columns = QString::fromStdString(m_value).split("/");
  columns[column] = QString::fromStdString(value);
  set_value(columns.join("/").toStdString());
}

std::string PreferencesTreeValueItem::value(std::size_t column) const
{
  LINFO << m_value;
  return QString::fromStdString(m_value).split("/")[column].toStdString();
}

void PreferencesTreeValueItem::set_column_count(std::size_t c)
{
  m_value = QString("/").repeated(c-1).toStdString();
}

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const std::string& group)
  : PreferencesTreeItem(group)
{
}



}  // namespace omm
