#include "keybindings/keybinding.h"
#include <QSettings>
#include "logging.h"

namespace omm
{

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const QKeySequence& sequence)
  : KeyBindingTreeItem(name), m_context(context)
  , m_sequence(sequence), m_default_sequence(sequence)
{
}

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const std::string& sequence)
  : KeyBinding( name, context,
                QKeySequence(QString::fromStdString(sequence), QKeySequence::PortableText)) { }

void KeyBinding::set_key_sequence(const QKeySequence& sequence)
{
  if (m_sequence != sequence) {
    m_sequence = sequence;
    Q_EMIT key_sequence_changed(m_sequence);
  }
}

QKeySequence KeyBinding::key_sequence() const { return m_sequence; }
QKeySequence KeyBinding::default_key_sequence() const { return m_default_sequence; }
std::string KeyBinding::context() const { return m_context; }

bool KeyBinding::matches(QKeySequence sequence) const
{
   return sequence.count() > 0 && m_sequence.matches(sequence) == QKeySequence::ExactMatch;
}

void KeyBinding::reset()
{
  m_sequence = m_default_sequence;
}

ContextKeyBindings::ContextKeyBindings(const std::string& name) : KeyBindingTreeItem(name) {}

KeyBindingTreeItem::KeyBindingTreeItem(const std::string& name) : name(name) {}

}  // namespace omm
