#include "keybindings/action.h"
#include <memory>
#include "aspects/settingstreeitem.h"
#include <QWidget>
#include "common.h"
#include "logging.h"
#include "keybindings/actionwidget.h"

namespace omm
{

Action::Action(const SettingsTreeValueItem& key_binding)
  : QWidgetAction(nullptr), m_key_binding(key_binding)
{
}

QWidget* Action::createWidget(QWidget* parent)
{
  auto aw = std::make_unique<ActionWidget>(parent, m_key_binding);
  aw->setEnabled(isEnabled());
  m_action_widget = aw.get();
  return aw.release();
}

void Action::set_highlighted(bool h)
{
  if (m_action_widget != nullptr) { m_action_widget->set_highlighted(h); }
}

}  // namespace omm
