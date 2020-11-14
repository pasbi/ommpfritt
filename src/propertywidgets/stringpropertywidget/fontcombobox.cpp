#include "propertywidgets/stringpropertywidget/fontcombobox.h"

namespace omm
{
FontComboBox::FontComboBox(QWidget* parent) : QFontComboBox(parent)
{
  setFocusPolicy(Qt::StrongFocus);
}

void FontComboBox::wheelEvent(QWheelEvent* e)
{
  if (hasFocus()) {
    QFontComboBox::wheelEvent(e);
  }
}

}  // namespace omm
