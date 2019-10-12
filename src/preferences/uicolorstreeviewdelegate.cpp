#include "preferences/uicolorstreeviewdelegate.h"

namespace omm
{

void UiColorsTreeViewDelegate::set_editor_data(UiColorEdit& editor, const QModelIndex& index) const
{
  const Color color = Color(index.data(Qt::EditRole).toString().toStdString());
  editor.set_color(color);
}

bool UiColorsTreeViewDelegate::set_model_data(UiColorEdit& editor, QAbstractItemModel& model,
                                               const QModelIndex& index) const
{
  return model.setData(index, editor.color().to_qcolor(), Qt::EditRole);
}

}  // namespace omm
