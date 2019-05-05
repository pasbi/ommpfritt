#pragma once

#include <QKeySequence>

class QSettings;

namespace omm
{

class KeyBinding
{
public:
  KeyBinding(const std::string& name, const std::string context, const QKeySequence& sequence);
  KeyBinding(const std::string& name, const std::string context, const std::string& sequence);
  void set_key_sequence(const QKeySequence& sequence);
  bool matches(const QKeySequence& sequence, const std::string& context) const;
  std::string name() const;
  QKeySequence key_sequence() const;
  QKeySequence default_key_sequence() const;
  std::string context() const;
  bool collides_with(const KeyBinding& other) const;

private:
  const std::string m_name;
  const std::string m_context;
  QKeySequence m_sequence;
  QKeySequence m_default_sequence;
};

}  // namespace omm
