#include "propertywidgets/stringpropertywidget/texteditadapter.h"

namespace omm
{
// AbstractTextEditAdapter
// TextEditAdapter<QLineEdit>

void TextEditAdapter<QLineEdit>::set_inconsistent_value()
{
  QLineEdit::setPlaceholderText(QObject::tr("< multiple values >", "TextEditAdapter"));
  QLineEdit::clear();
}

void TextEditAdapter<QLineEdit>::set_value(const QString& text)
{
  QLineEdit::setText(text);
}

void TextEditAdapter<QLineEdit>::clear()
{
  QLineEdit::clear();
}
QString TextEditAdapter<QLineEdit>::value() const
{
  return QLineEdit::text();
}
QWidget* TextEditAdapter<QLineEdit>::as_widget()
{
  return this;
}

// TextEditAdapter<QTextEdit>

TextEditAdapter<QTextEdit>::TextEditAdapter(QWidget* parent) : QTextEdit(parent)
{
  static constexpr int TAB_STOP_DISTANCE = 20;
  setTabStopDistance(TAB_STOP_DISTANCE);
}

void TextEditAdapter<QTextEdit>::set_inconsistent_value()
{
  QTextEdit::setPlaceholderText(QObject::tr("< multiple values >", "TextEditAdapter"));
  QTextEdit::clear();
}

void TextEditAdapter<QTextEdit>::set_value(const QString& text)
{
  if (text != this->value()) {
    QTextEdit::setText(text);
  }
}

void TextEditAdapter<QTextEdit>::clear()
{
  QTextEdit::clear();
}

QString TextEditAdapter<QTextEdit>::value() const
{
  return QTextEdit::toPlainText();
}

QWidget* TextEditAdapter<QTextEdit>::as_widget()
{
  return this;
}

// TextEditAdapter<FilePathEdit>

void TextEditAdapter<FilePathEdit>::set_inconsistent_value()
{
  const auto placeholder_text = QObject::tr("< multiple values >", "TextEditAdapter");
  FilePathEdit::set_placeholder_text(placeholder_text);
  FilePathEdit::clear();
}

void TextEditAdapter<FilePathEdit>::set_value(const QString& text)
{
  FilePathEdit::set_path(text);
}

void TextEditAdapter<FilePathEdit>::clear()
{
  FilePathEdit::clear();
}
QString TextEditAdapter<FilePathEdit>::value() const
{
  return FilePathEdit::path();
}
QWidget* TextEditAdapter<FilePathEdit>::as_widget()
{
  return this;
}

// TextEditAdapter<CodeEdit>

void TextEditAdapter<CodeEdit>::set_inconsistent_value()
{
  const auto placeholder_text = QObject::tr("< multiple values >", "TextEditAdapter");
  CodeEdit::set_placeholder_text(placeholder_text);
}

void TextEditAdapter<CodeEdit>::set_value(const QString& text)
{
  CodeEdit::set_code(text);
}
void TextEditAdapter<CodeEdit>::clear()
{
  CodeEdit::clear();
}
QString TextEditAdapter<CodeEdit>::value() const
{
  return CodeEdit::code();
}
QWidget* TextEditAdapter<CodeEdit>::as_widget()
{
  return this;
}

// TextEditAdapter<QFontComboBox>

void TextEditAdapter<FontComboBox>::set_value(const QString& text)
{
  QFontComboBox::setCurrentFont(QFont(text));
}

void TextEditAdapter<FontComboBox>::set_inconsistent_value()
{
  setCurrentIndex(-1);
}
void TextEditAdapter<FontComboBox>::clear()
{
  FontComboBox::clear();
}
QString TextEditAdapter<FontComboBox>::value() const
{
  return currentText();
}
QWidget* TextEditAdapter<FontComboBox>::as_widget()
{
  return this;
}

}  // namespace omm
