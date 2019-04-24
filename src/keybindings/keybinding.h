#pragma once

#include <QKeySequence>

class QSettings;

namespace omm
{

class KeyBinding
{
public:
  KeyBinding( const std::string& name, const std::string context,
              const QKeySequence& sequence = QKeySequence() );
  KeyBinding( const std::string& name, const std::string context,
              const std::string& sequence = "" );
  void set_key_sequence(const QKeySequence& sequence);
  bool matches(const QKeySequence& sequence, const std::string& context) const;
  std::string name() const;
  QKeySequence key_sequence() const;
  std::string context() const;

private:
  const std::string m_name;
  const std::string m_context;
  QKeySequence m_sequence;
};

}  // namespace omm
