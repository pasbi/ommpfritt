#include "preferences/preferencestreeitem.h"
#include "logging.h"

namespace omm
{

PreferencesTreeItem::PreferencesTreeItem(const std::string& name) : name(name) { }

PreferencesTreeValueItem::
PreferencesTreeValueItem(const std::string& group, const std::string& name, const std::string& value)
  : PreferencesTreeItem(name), group(group), m_value(value), m_default(value)
{
}

void PreferencesTreeValueItem::set_default(const std::string& value)
{
  m_default = value;
}

void PreferencesTreeValueItem::set_value(const std::string& value)
{
  if (value!= m_value) {
    m_value = value;
    Q_EMIT(value_changed(m_value));
  }
}

void PreferencesTreeValueItem::set_value(const std::string& value, std::size_t column)
{
  QStringList columns = QString::fromStdString(m_value).split("/");
  columns[column] = QString::fromStdString(value);
  set_value(columns.join("/").toStdString());
}

std::string PreferencesTreeValueItem::value(std::size_t column) const
{
  return QString::fromStdString(m_value).split("/")[column].toStdString();
}

std::string PreferencesTreeValueItem::value() const
{
  return m_value;
}

std::string PreferencesTreeValueItem::default_value(std::size_t column) const
{
  return QString::fromStdString(m_default).split("/")[column].toStdString();
}

void PreferencesTreeValueItem::reset()
{
  set_value(m_default);
}

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const std::string& group)
  : PreferencesTreeItem(group)
{
}



}  // namespace omm
