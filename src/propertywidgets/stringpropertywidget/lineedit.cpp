#include "propertywidgets/stringpropertywidget/lineedit.h"

namespace omm
{

LineEdit::LineEdit()
{
  connect(this, &QLineEdit::textChanged, [this]() { setPlaceholderText(""); });
}

void LineEdit::set_value(const value_type& value)
{
  setText(QString::fromStdString(value));
}

void LineEdit::set_inconsistent_value()
{
  setPlaceholderText(tr("<multiple values>"));
  clear();
}

LineEdit::value_type LineEdit::value() const
{
  return text().toStdString();
}

}  // namespace omm
