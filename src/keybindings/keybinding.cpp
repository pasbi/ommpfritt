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
  return m_context == other.m_context && m_sequence == other.m_sequence;
}

}  // namespace omm
