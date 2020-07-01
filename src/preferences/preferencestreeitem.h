#pragma once

#include <QIcon>
#include <QObject>
#include <string>
#include <memory>

namespace omm
{

class PreferencesTreeItem
{
protected:
  explicit PreferencesTreeItem(const QString& name, const QString& translation_context);
  virtual ~PreferencesTreeItem() = default;
public:
  virtual bool is_group() const = 0;
  const QString name;
  virtual QString translated_name() const = 0;

protected:
  const QString translation_context;
};

class PreferencesTreeValueItem: public QObject, public PreferencesTreeItem
{
  Q_OBJECT
public:
  PreferencesTreeValueItem(const QString& group, const QString& name,
                           const QString& value, const QString& translation_context);
  void set_default(const QString& value);
  void set_value(const QString& value);
  void set_value(const QString& value, std::size_t column);
  QString value(std::size_t column) const;
  QString value() const;
  QString default_value(std::size_t column = 0) const;
  bool is_group() const override { return false; }
  void reset();
  QString translated_name() const override;
  QIcon icon() const;

  const QString group;
  static QString value(const QString& value, std::size_t column);

Q_SIGNALS:
  void value_changed(const QString& value);

private:
  QString m_value;
  QString m_default;
};

class PreferencesTreeGroupItem : public PreferencesTreeItem
{
public:
  explicit PreferencesTreeGroupItem(const QString& group, const QString& translation_context);
  std::vector<std::unique_ptr<PreferencesTreeValueItem>> values;
  bool is_group() const override { return true; }
  QString translated_name() const override;
};

}  // namespace omm
