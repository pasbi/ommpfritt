#pragma once

#include <QObject>
#include <string>
#include <memory>

namespace omm
{

class PreferencesTreeItem
{
protected:
  explicit PreferencesTreeItem(const std::string& name);
  virtual ~PreferencesTreeItem() = default;
public:
  virtual bool is_group() const = 0;
  const std::string name;
};

class PreferencesTreeValueItem: public QObject, public PreferencesTreeItem
{
  Q_OBJECT
public:
  PreferencesTreeValueItem(const std::string& group, const std::string& name,
                           const std::string& value);
  void set_default(const std::string& value);
  void set_value(const std::string& value);
  void set_value(const std::string& value, std::size_t column);
  std::string value(std::size_t column) const;
  std::string value() const;
  std::string default_value(std::size_t column = 0) const;
  bool is_group() const override { return false; }
  void reset();

  const std::string group;

Q_SIGNALS:
  void value_changed(const std::string& value);

private:
  std::string m_value;
  std::string m_default;
};

class PreferencesTreeGroupItem : public PreferencesTreeItem
{
public:
  explicit PreferencesTreeGroupItem(const std::string& group);
  std::vector<std::unique_ptr<PreferencesTreeValueItem>> values;
  bool is_group() const override { return true; }
};

}  // namespace omm
