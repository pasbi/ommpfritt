#include "preferences/preferencestreeitem.h"
#include <QCoreApplication>
#include "logging.h"

namespace omm
{

PreferencesTreeItem::PreferencesTreeItem(const QString& name) : name(name) { }

PreferencesTreeValueItem::
PreferencesTreeValueItem(const QString& group, const QString& name, const QString& value)
  : PreferencesTreeItem(name), group(group), m_value(value), m_default(value)
{
}

void PreferencesTreeValueItem::set_default(const QString& value)
{
  m_default = value;
}

void PreferencesTreeValueItem::set_value(const QString& value)
{
  if (value!= m_value) {
    m_value = value;
    Q_EMIT(value_changed(m_value));
  }
}

void PreferencesTreeValueItem::set_value(const QString& value, std::size_t column)
{
  QStringList columns = m_value.split("/");
  columns[column] = value;
  set_value(columns.join("/"));
}

QString PreferencesTreeValueItem::value(std::size_t column) const
{
  return value(m_value, column);
}

QString PreferencesTreeValueItem::value() const
{
  return m_value;
}

QString PreferencesTreeValueItem::default_value(std::size_t column) const
{
  return value(m_default, column);
}

void PreferencesTreeValueItem::reset()
{
  set_value(m_default);
}

QString PreferencesTreeValueItem::translated_name(const QString& context) const
{
  return QCoreApplication::translate((context + "/" + group).toUtf8().constData(),
                                     name.toUtf8().constData());
}

QString PreferencesTreeValueItem::value(const QString& value, std::size_t column)
{
  return value.split("/")[column];
}

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const QString& group)
  : PreferencesTreeItem(group)
{
}

QString PreferencesTreeGroupItem::translated_name(const QString& context) const
{
  return QCoreApplication::translate((context + "/" + name).toUtf8().constData(),
                                     name.toUtf8().constData());
}



}  // namespace omm
