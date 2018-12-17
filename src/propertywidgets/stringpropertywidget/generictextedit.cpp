#include "propertywidgets/stringpropertywidget/generictextedit.h"

namespace omm
{

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
  connect(m_text_edit, SIGNAL(textChanged(QString)), this, SIGNAL(text_changed()));
}

std::string SingleLineTextEdit::value() const
{
  return m_text_edit->text().toStdString();
}

}  // namespace omm

