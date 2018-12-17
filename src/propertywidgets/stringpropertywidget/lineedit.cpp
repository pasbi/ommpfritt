#include "propertywidgets/stringpropertywidget/lineedit.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

namespace omm
{

LineEdit::LineEdit(StringProperty::LineMode mode)
{

  setLayout(std::make_unique<QVBoxLayout>(this).release());

  switch (mode) {
  case StringProperty::LineMode::SingleLine:
    m_line_edit = std::make_unique<QLineEdit>(this).release();
    layout()->addWidget(m_line_edit);
    connect(m_line_edit, &QLineEdit::textChanged, [this]() {
      m_line_edit->setPlaceholderText("");
    });
    connect(m_line_edit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    break;
  case StringProperty::LineMode::MultiLine:
    m_text_edit = std::make_unique<QTextEdit>(this).release();
    layout()->addWidget(m_text_edit);
    connect(m_text_edit, &QTextEdit::textChanged, [this]() {
      m_text_edit->setPlaceholderText("");
    });
    connect(m_text_edit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    break;
  }
}

void LineEdit::set_value(const value_type& value)
{
  if (m_line_edit != nullptr) {
    m_line_edit->setText(QString::fromStdString(value));
  } else if (m_text_edit != nullptr) {
    m_text_edit->setText(QString::fromStdString(value));
  } else {
    assert(false);
  }
}

void LineEdit::set_inconsistent_value()
{
  if (m_line_edit != nullptr) {
    m_line_edit->setPlaceholderText(tr("<multiple values>"));
    m_line_edit->clear();
  } else if (m_text_edit != nullptr) {
    m_text_edit->setPlaceholderText(tr("<multiple values>"));
    m_text_edit->clear();
  } else {
    assert(false);
  }
}

LineEdit::value_type LineEdit::value() const
{
  if (m_line_edit != nullptr) {
    return m_line_edit->text().toStdString();
  } else if (m_text_edit != nullptr) {
    return m_text_edit->toPlainText().toStdString();
  } else {
    assert(false);
  }
}

}  // namespace omm
