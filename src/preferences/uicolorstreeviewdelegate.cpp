#include "preferences/uicolorstreeviewdelegate.h"
#include "logging.h"
#include "preferences/preferencestree.h"
#include "uicolors.h"
#include <QPainter>

namespace omm
{
void UiColorsTreeViewDelegate::set_editor_data(UiColorEdit& editor, const QModelIndex& index) const
{
  const Color color = Color::from_html(index.data(Qt::EditRole).toString());
  const auto default_value = index.data(PreferencesTree::DEFAULT_VALUE_ROLE).toString();
  editor.set_default_color(Color::from_html(default_value));
  editor.set_color(color);
}

bool UiColorsTreeViewDelegate::set_model_data(UiColorEdit& editor,
                                              QAbstractItemModel& model,
                                              const QModelIndex& index) const
{
  return model.setData(index, editor.color().to_qcolor(), Qt::EditRole);
}

void UiColorsTreeViewDelegate::paint(QPainter* painter,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
  assert(index.isValid());
  const QString html = index.data(Qt::EditRole).toString();
  if (html.isEmpty()) {
    painter->fillRect(option.rect, ui_color(*option.widget, QPalette::Base));
  } else {
    const Color color = Color::from_html(html);
    painter->save();
    painter->translate(option.rect.topLeft());
    const QPoint tl = option.rect.topLeft();
    const auto rect = option.rect.translated(-tl);
    UiColors::draw_background(*painter, rect);
    painter->fillRect(rect, color.to_qcolor());
    painter->restore();
  }
}

}  // namespace omm
