#include "keybindings/modeselector.h"
#include "keybindings/commandinterface.h"
#include "keybindings/keybindings.h"
#include <QCoreApplication>

namespace omm
{
ModeSelector::ModeSelector(CommandInterface& context,
                           const QString& name,
                           const QString& cycle_action,
                           const std::vector<QString>& activation_actions)
    : context(context), name(name), cycle_action(cycle_action),
      activation_actions(activation_actions)
{
}

ModeSelector::~ModeSelector() = default;

QString ModeSelector::translated_name() const
{
  QString ctx_string = KeyBindings::TRANSLATION_CONTEXT;
  ctx_string += "/";
  ctx_string += context.type();
  const QByteArray ctx = ctx_string.toUtf8();
  const QByteArray name = this->name.toUtf8();
  return QCoreApplication::translate(ctx.constData(), name.constData());
}

bool ModeSelector::handle(const QString& action_name)
{
  const auto prefix = this->name + ".";
  if (action_name.startsWith(prefix)) {
    if (action_name == cycle_action) {
      cycle();
    } else {
      const auto it = std::find(activation_actions.begin(), activation_actions.end(), action_name);
      assert(it != activation_actions.end());
      set_mode(std::distance(activation_actions.begin(), it));
    }
    return true;
  } else {
    return false;
  }
}

void ModeSelector::set_mode(int mode)
{
  if (m_mode != mode) {
    m_mode = mode;
    Q_EMIT mode_changed(mode);
  }
}

void ModeSelector::cycle()
{
  set_mode((m_mode + 1) % static_cast<int>(activation_actions.size()));
}

}  // namespace omm
