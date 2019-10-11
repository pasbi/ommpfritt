#include "preferences/keybindingstreeviewdelegate.h"
#include <QHeaderView>
#include "keybindings/keybindings.h"
#include "logging.h"
#include "widgets/keysequenceedit.h"

namespace omm
{

void KeyBindingsTreeViewDelegate::set_editor_data(omm::KeySequenceEdit &editor,
                                              const QModelIndex &index) const
{
  editor.set_default_key_sequence(index.data(omm::KeyBindings::DEFAULT_KEY_SEQUENCE_ROLE)
                                      .value<QKeySequence>());
  editor.set_key_sequence(index.data(Qt::EditRole).value<QKeySequence>());
}

bool KeyBindingsTreeViewDelegate::set_model_data(omm::KeySequenceEdit &editor,
                                                 QAbstractItemModel &model,
                                                 const QModelIndex &index) const
{
  return model.setData(index, editor.key_sequence(), Qt::EditRole);
}

}  // namespace omm
