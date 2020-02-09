#include "widgets/numericedit.h"

namespace omm
{

AbstractNumericEdit::AbstractNumericEdit(QWidget* parent) : QLineEdit(parent)
{
  setReadOnly(true);
}

void AbstractNumericEdit::paintEvent(QPaintEvent* e)
{
  QLineEdit::paintEvent(e);
}

void AbstractNumericEdit::focusOutEvent(QFocusEvent* e)
{
  setReadOnly(true);
  QLineEdit::focusOutEvent(e);
}

void AbstractNumericEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  setReadOnly(false);
  selectAll();
}

}  // namespace omm

