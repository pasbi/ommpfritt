#include "propertywidgets/stringpropertywidget/texteditadapter.h"

namespace omm
{

AbstractTextEditAdapter
::AbstractTextEditAdapter(const on_value_changed_t& on_value_changed)
  : MultiValueEdit<std::string>(on_value_changed)
{ }

TextEditAdapter<QLineEdit>
::TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent)
  : QLineEdit(parent), AbstractTextEditAdapter(on_value_changed)
{
  connect(this, &QLineEdit::textChanged, [on_value_changed, this]() {
    if (!signalsBlocked()) { on_value_changed(value()); }
  });
}

void TextEditAdapter<QLineEdit>::set_inconsistent_value()
{
  QLineEdit::setPlaceholderText(QObject::tr("< multiple values >", "TextEditAdapter"));
  QLineEdit::clear();
}

void TextEditAdapter<QLineEdit>::set_value(const std::string& text)
{
  QLineEdit::setText(QString::fromStdString(text));
}

void TextEditAdapter<QLineEdit>::clear() { QLineEdit::clear(); }
std::string TextEditAdapter<QLineEdit>::value() const { return QLineEdit::text().toStdString(); }
QWidget* TextEditAdapter<QLineEdit>::as_widget() { return this; }

TextEditAdapter<QTextEdit>
::TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent)
  : QTextEdit(parent), AbstractTextEditAdapter(on_value_changed)
{
  connect(this, &QTextEdit::textChanged, [on_value_changed, this]() {
    if (!signalsBlocked()) { on_value_changed(value()); }
  });
}

void TextEditAdapter<QTextEdit>::set_inconsistent_value()
{
  QTextEdit::setPlaceholderText(QObject::tr("< multiple values >", "TextEditAdapter"));
  QTextEdit::clear();
}

void TextEditAdapter<QTextEdit>::set_value(const std::string& text)
{
  if (text != this->value()) {
    QTextEdit::setText(QString::fromStdString(text));
  }
}

void TextEditAdapter<QTextEdit>::clear() { QTextEdit::clear(); }

std::string TextEditAdapter<QTextEdit>::value() const
{
  return QTextEdit::toPlainText().toStdString();
}

QWidget* TextEditAdapter<QTextEdit>::as_widget() { return this; }

TextEditAdapter<FilePathEdit>
::TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent)
  : FilePathEdit(parent), AbstractTextEditAdapter(on_value_changed)
{
  connect(this, &FilePathEdit::path_changed, [on_value_changed, this]() {
    if (!signalsBlocked()) { on_value_changed(value()); }
  });
}

void TextEditAdapter<FilePathEdit>::set_inconsistent_value()
{
  const auto placeholder_text = QObject::tr("< multiple values >", "TextEditAdapter");
  FilePathEdit::set_placeholder_text(placeholder_text.toStdString());
  FilePathEdit::clear();
}

void TextEditAdapter<FilePathEdit>::set_value(const std::string& text)
{
  FilePathEdit::set_path(text);
}

void TextEditAdapter<FilePathEdit>::clear() { FilePathEdit::clear(); }
std::string TextEditAdapter<FilePathEdit>::value() const { return FilePathEdit::path(); }

QWidget* TextEditAdapter<FilePathEdit>::as_widget() { return this; }


TextEditAdapter<CodeEdit>
::TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent)
  : CodeEdit(parent), AbstractTextEditAdapter(on_value_changed)
{
  connect(this, &CodeEdit::code_changed, [on_value_changed, this]() {
    if (!signalsBlocked()) { on_value_changed(value()); }
  });
}

void TextEditAdapter<CodeEdit>::set_inconsistent_value()
{
  const auto placeholder_text = QObject::tr("< multiple values >", "TextEditAdapter");
  CodeEdit::set_placeholder_text(placeholder_text.toStdString());
}

void TextEditAdapter<CodeEdit>::set_value(const std::string& text) { CodeEdit::set_code(text); }
void TextEditAdapter<CodeEdit>::clear() { CodeEdit::clear(); }
std::string TextEditAdapter<CodeEdit>::value() const { return CodeEdit::code(); }

QWidget* TextEditAdapter<CodeEdit>::as_widget() { return this; }

}  // namespace omm

