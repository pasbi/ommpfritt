#include "propertywidgets/optionspropertywidget/optionsedit.h"
#include "common.h"
#include <QPaintEvent>
#include <QStylePainter>

namespace omm
{

void OptionsEdit::set_value(const value_type& value) { setCurrentIndex(value); }
void OptionsEdit::set_inconsistent_value() { setCurrentIndex(-1); }
OptionsEdit::value_type OptionsEdit::value() const { return currentIndex(); }

void OptionsEdit::set_options(const std::vector<QString>& options)
{
  clear();
  addItems(::transform<QString, QList>(options, ::identity));
}

void PrefixComboBox::paintEvent(QPaintEvent*)
{
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));
  // draw the combobox frame, focusrect and selected etc.
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  painter.drawComplexControl(QStyle::CC_ComboBox, opt);
  // draw the icon and text
  opt.currentText = tr("%1: %2").arg(prefix, currentText());
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

}  // namespace omm
