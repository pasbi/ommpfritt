#include "preferences/keybindingstreeviewdelegate.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "preferences/keysequenceedit.h"
#include <QHeaderView>

namespace omm
{
void KeyBindingsTreeViewDelegate::set_editor_data(omm::KeySequenceEdit& editor,
                                                  const QModelIndex& index) const
{
  editor.set_default_key_sequence(
      index.data(KeyBindings::DEFAULT_VALUE_ROLE).value<QKeySequence>());
  editor.set_key_sequence(index.data(Qt::EditRole).value<QKeySequence>());
}

bool KeyBindingsTreeViewDelegate::set_model_data(omm::KeySequenceEdit& editor,
                                                 QAbstractItemModel& model,
                                                 const QModelIndex& index) const
{
  return model.setData(index, editor.key_sequence(), Qt::EditRole);
}

}  // namespace omm
