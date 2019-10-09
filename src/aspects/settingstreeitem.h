#pragma once

#include <QObject>
#include <string>
#include <memory>

namespace omm
{

class SettingsTreeItem
{
public:
  std::map<std::string, std::string> user_data;
protected:
  explicit SettingsTreeItem(const std::string& name);
  virtual ~SettingsTreeItem() = default;
public:
  virtual bool is_group() const = 0;
  const std::string name;
};

class SettingsTreeValueItem : public QObject, public SettingsTreeItem
{
  Q_OBJECT
public:
  SettingsTreeValueItem(const std::string& group, const std::string& name, const std::string& value);
  void set_value(const std::string& value);
  std::string value() const;
  bool is_group() const override { return false; }

  const std::string group;

Q_SIGNALS:
  void value_changed(const std::string& sequence);

private:
  std::string m_value;
};

class SettingsTreeGroupItem : public SettingsTreeItem
{
public:
  explicit SettingsTreeGroupItem(const std::string& group);
  std::vector<std::unique_ptr<SettingsTreeValueItem>> values;
  bool is_group() const override { return true; }
};

}  // namespace omm
