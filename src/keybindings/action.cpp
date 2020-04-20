#include "keybindings/action.h"
#include "preferences/preferencestreeitem.h"
#include "logging.h"
#include "keybindings/keybindings.h"

namespace omm
{

Action::Action(const PreferencesTreeValueItem& key_binding)
{
  setIcon(key_binding.icon());
  setText(key_binding.translated_name(KeyBindings::TRANSLATION_CONTEXT));

  connect(&key_binding, &PreferencesTreeValueItem::value_changed, this, [this](const QString& s) {
    setShortcut(QKeySequence(s));
  });
  setShortcut(QKeySequence(key_binding.value()));
}

}  // namespace omm
