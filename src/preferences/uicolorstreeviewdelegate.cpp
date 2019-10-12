#include "preferences/uicolorstreeviewdelegate.h"

namespace omm
{

void UiColorsTreeViewDelegate::set_editor_data(UiColorEdit& editor, const QModelIndex& index) const
{
  editor.set_color(index.data(Qt::EditRole).value<QColor>());
}

bool UiColorsTreeViewDelegate::set_model_data(UiColorEdit& editor, QAbstractItemModel& model,
                                               const QModelIndex& index) const
{
  return model.setData(index, QColor(editor.color()), Qt::EditRole);
}

}  // namespace omm
