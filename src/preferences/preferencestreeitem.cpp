#include "preferences/preferencestreeitem.h"
#include <QCoreApplication>
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
  return value(m_value, column);
}

std::string PreferencesTreeValueItem::value() const
{
  return m_value;
}

std::string PreferencesTreeValueItem::default_value(std::size_t column) const
{
  return value(m_default, column);
}

void PreferencesTreeValueItem::reset()
{
  set_value(m_default);
}

std::string PreferencesTreeValueItem::translated_name(const std::string& context) const
{
  return QCoreApplication::translate((context + "/" + group).c_str(),
                                     name.c_str()).toStdString();
}

std::string PreferencesTreeValueItem::value(const std::string& value, std::size_t column)
{
  return QString::fromStdString(value).split("/")[column].toStdString();
}

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const std::string& group)
  : PreferencesTreeItem(group)
{
}

std::string PreferencesTreeGroupItem::translated_name(const std::string& context) const
{
  return QCoreApplication::translate((context + "/" + name).c_str(),
                                     name.c_str()).toStdString();
}



}  // namespace omm
