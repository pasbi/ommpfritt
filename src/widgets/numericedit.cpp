#include "widgets/numericedit.h"

namespace omm
{

void AbstractNumericEdit::paintEvent(QPaintEvent* e)
{
  QSignalBlocker blocker(this);
  const QString old_text = text();
  QString new_text = label.isEmpty() ? old_text : tr("%1: %2").arg(label, old_text);
  new_text = fontMetrics().elidedText(new_text, Qt::ElideRight, width());
  setText(new_text);
  QLineEdit::paintEvent(e);
  setText(old_text);
}

}  // namespace omm

