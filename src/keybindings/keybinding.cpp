#include "keybindings/keybinding.h"
#include <QSettings>
#include "logging.h"

namespace omm
{

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const QKeySequence& sequence)
  : m_name(name), m_context(context), m_sequence(sequence), m_default_sequence(sequence) { }

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const std::string& sequence)
  : KeyBinding( name, context,
                QKeySequence(QString::fromStdString(sequence), QKeySequence::PortableText)) { }

bool KeyBinding::matches(const QKeySequence& sequence, const std::string& context) const
{
  return m_sequence.matches(sequence) == QKeySequence::ExactMatch && m_context == context;
}

void KeyBinding::set_key_sequence(const QKeySequence& sequence) { m_sequence = sequence; }
std::string KeyBinding::name() const { return m_name; }
QKeySequence KeyBinding::key_sequence() const { return m_sequence; }
QKeySequence KeyBinding::default_key_sequence() const { return m_default_sequence; }
std::string KeyBinding::context() const { return m_context; }

bool KeyBinding::collides_with(const KeyBinding &other) const
{
  if (m_context != other.m_context) {
    return false;
  }

  if (m_sequence.isEmpty() || other.m_sequence.isEmpty()) {
    return false;
  }

  for (int i = 0; i < m_sequence.count(); ++i) {
    if (m_sequence[i] != other.m_sequence[i]) {
      return false;
    }
  }
  return true;
}

}  // namespace omm
