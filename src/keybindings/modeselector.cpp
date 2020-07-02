#include "keybindings/modeselector.h"

namespace omm
{

ModeSelector::ModeSelector(const QString& cycle_action,
                           const std::vector<QString>& activation_actions)
  : cycle_action(cycle_action), activation_actions(activation_actions)
{

}

ModeSelector::~ModeSelector()
{
}

void ModeSelector::set_mode(int mode)
{
  if (m_mode != mode) {
    m_mode = mode;
    Q_EMIT mode_changed(mode);
  }
}

void ModeSelector::on_action(const QString& action)
{
  if (action == cycle_action) {
    cycle();
  } else {
    const auto it = std::find(activation_actions.begin(), activation_actions.end(), action);
    assert(it != activation_actions.end());
    set_mode(std::distance(activation_actions.begin(), it));
  }
}

void ModeSelector::cycle()
{
  set_mode((m_mode + 1) % activation_actions.size());
}

}  // namespace omm
