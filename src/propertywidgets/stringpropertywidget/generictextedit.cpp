#include "propertywidgets/stringpropertywidget/generictextedit.h"

namespace omm
{

AbstractGenericTextEdit::
AbstractGenericTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed)
  : QWidget(parent), MultiValueEdit<std::string>(on_value_changed) { }

MultiLineTextEdit::MultiLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed)
  : GenericTextEdit<QTextEdit>(parent, on_value_changed)
{
  setFont(QFont("Courier", 12));
  connect(m_text_edit, &QTextEdit::textChanged, [f=on_value_changed, this]() {
    if (!signalsBlocked()) { f(value()); }
  });
}

std::string MultiLineTextEdit::value() const { return m_text_edit->toPlainText().toStdString(); }

SingleLineTextEdit::SingleLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed)
  : GenericTextEdit<QLineEdit>(parent, on_value_changed)
{
  connect(m_text_edit, &QLineEdit::textChanged, [f=on_value_changed, this]() {
    if (!signalsBlocked()) { (value()); }
  });
}

std::string SingleLineTextEdit::value() const { return m_text_edit->text().toStdString(); }

}  // namespace omm

