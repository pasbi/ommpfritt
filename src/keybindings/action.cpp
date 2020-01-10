#include "keybindings/action.h"
#include <memory>
#include "preferences/preferencestreeitem.h"
#include <QWidget>
#include "common.h"
#include "logging.h"
#include "keybindings/actionwidget.h"
#include "keybindings/keybindings.h"

namespace omm
{

Action::Action(const PreferencesTreeValueItem& key_binding)
  : QWidgetAction(nullptr), m_key_binding(key_binding)
{
  setIcon(key_binding.icon());
  setText(key_binding.translated_name(KeyBindings::TRANSLATION_CONTEXT));
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
