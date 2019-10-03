#include "keybindings/keybinding.h"
#include <QSettings>
#include "logging.h"

namespace omm
{

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const QKeySequence& sequence)
  : KeyBindingTreeItem(name), m_context(context)
  , m_sequence(sequence), m_default_sequence(sequence)\
{
}

KeyBinding::KeyBinding(const std::string& name, const std::string context,
                       const std::string& sequence)
  : KeyBinding( name, context,
                QKeySequence(QString::fromStdString(sequence), QKeySequence::PortableText)) { }

void KeyBinding::set_key_sequence(const QKeySequence& sequence) { m_sequence = sequence; }
QKeySequence KeyBinding::key_sequence() const { return m_sequence; }
QKeySequence KeyBinding::default_key_sequence() const { return m_default_sequence; }
std::string KeyBinding::context() const { return m_context; }

bool KeyBinding::matches(QKeySequence sequence) const
{
  if (sequence.count() == 0) {
    return false;
  } else if (m_sequence.matches(sequence) == QKeySequence::ExactMatch) {
    return true;
  } else {
    return matches(QKeySequence(sequence[1], sequence[2], sequence[3], 0));
  }
}

ContextKeyBindings::ContextKeyBindings(const std::string& name) : KeyBindingTreeItem(name) {}

KeyBindingTreeItem::KeyBindingTreeItem(const std::string& name) : name(name) {}

}  // namespace omm
