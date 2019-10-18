#include "preferences/uicolorstreeviewdelegate.h"
#include "logging.h"
#include "preferences/preferencestree.h"

namespace omm
{

void UiColorsTreeViewDelegate::set_editor_data(UiColorEdit& editor, const QModelIndex& index) const
{
  const Color color = Color::from_html(index.data(Qt::EditRole).toString().toStdString());
  const auto default_value = index.data(PreferencesTree::DEFAULT_VALUE_ROLE).toString();
  editor.set_default_color(Color::from_html(default_value.toStdString()));
  editor.set_color(color);
}

bool UiColorsTreeViewDelegate::set_model_data(UiColorEdit& editor, QAbstractItemModel& model,
                                               const QModelIndex& index) const
{
  return model.setData(index, editor.color().to_qcolor(), Qt::EditRole);
}

}  // namespace omm
