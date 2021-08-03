#include "preferences/preferencestreeitem.h"
#include "logging.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include <QCoreApplication>

namespace omm
{
PreferencesTreeItem::PreferencesTreeItem(const QString& name, const QString& translation_context)
    : name(name), translation_context(translation_context)
{
}

PreferencesTreeValueItem::PreferencesTreeValueItem(const QString& group,
                                                   const QString& name,
                                                   const QString& value,
                                                   const QString& translation_context)
    : PreferencesTreeItem(name, translation_context), group(group), m_value(value), m_default(value)
{
}

void PreferencesTreeValueItem::set_default(const QString& value)
{
  m_default = value;
}

void PreferencesTreeValueItem::set_value(const QString& value)
{
  if (value != m_value) {
    m_value = value;
    Q_EMIT value_changed(m_value);
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

QString PreferencesTreeValueItem::translated_name() const
{
  const QByteArray context = (translation_context + "/" + group).toUtf8();
  const QByteArray text = name.toUtf8();
  return QCoreApplication::translate(context.constData(), text.constData());
}

QIcon PreferencesTreeValueItem::icon() const
{
  return IconProvider::pixmap(name);
}

QString PreferencesTreeValueItem::value(const QString& value, std::size_t column)
{
  return value.split("/")[column];
}

PreferencesTreeGroupItem::PreferencesTreeGroupItem(const QString& group,
                                                   const QString& translation_context)
    : PreferencesTreeItem(group, translation_context)
{
}

QString PreferencesTreeGroupItem::translated_name() const
{
  const auto context = translation_context + "/" + name;
  return QCoreApplication::translate(context.toUtf8().constData(), name.toUtf8().constData());
}

}  // namespace omm
