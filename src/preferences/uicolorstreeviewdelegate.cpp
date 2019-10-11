#include "preferences/uicolorstreeviewdelegate.h"

namespace omm
{

void UiColorsTreeViewDelegate::set_editor_data(QPushButton& editor, const QModelIndex& index) const
{

}

bool UiColorsTreeViewDelegate::set_model_data(QPushButton& editor, QAbstractItemModel& model,
                                               const QModelIndex& index) const
{
  return true;
}

}  // namespace omm
