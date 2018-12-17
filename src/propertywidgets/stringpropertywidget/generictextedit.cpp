#include "propertywidgets/stringpropertywidget/generictextedit.h"

namespace omm
{
  // TODO use same technique as in Property::get_value for observer

MultiLineTextEdit::MultiLineTextEdit(QWidget* parent)
  : GenericTextEdit<QTextEdit>(parent)
{
  connect(m_text_edit, SIGNAL(textChanged()), this, SIGNAL(text_changed()));
}

std::string MultiLineTextEdit::value() const
{
  return m_text_edit->toPlainText().toStdString();
}

SingleLineTextEdit::SingleLineTextEdit(QWidget* parent)
  : GenericTextEdit<QLineEdit>(parent)
{
  connect(m_text_edit, SIGNAL(textChanged()), this, SIGNAL(text_changed()));
}

std::string SingleLineTextEdit::value() const
{
  return m_text_edit->text().toStdString();
}

}  // namespace omm

