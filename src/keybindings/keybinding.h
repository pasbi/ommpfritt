#pragma once

#include <QKeySequence>
#include <QObject>
#include <memory>

class QSettings;

namespace omm
{

class KeyBindingTreeItem
{
protected:
  explicit KeyBindingTreeItem(const std::string& name);
  virtual ~KeyBindingTreeItem() = default;
public:
  virtual bool is_context() const = 0;
  const std::string name;
};

class KeyBinding : public QObject, public KeyBindingTreeItem
{
  Q_OBJECT
public:
  KeyBinding(const std::string& name, const std::string context, const QKeySequence& sequence);
  KeyBinding(const std::string& name, const std::string context, const std::string& sequence);
  void set_key_sequence(const QKeySequence& sequence);
  QKeySequence key_sequence() const;
  QKeySequence default_key_sequence() const;
  std::string context() const;
  bool is_context() const override { return false; }
  bool matches(QKeySequence sequence) const;
  void reset();

Q_SIGNALS:
  void key_sequence_changed(const QKeySequence& sequence);

private:
  const std::string m_context;
  QKeySequence m_sequence;
  QKeySequence m_default_sequence;
};

class ContextKeyBindings : public KeyBindingTreeItem
{
public:
  explicit ContextKeyBindings(const std::string& name);
  std::vector<std::unique_ptr<KeyBinding>> key_bindings;
  bool is_context() const override { return true; }
};

}  // namespace omm
