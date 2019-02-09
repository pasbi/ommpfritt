#include "widgets/codeedit.h"
#include <QHBoxLayout>
#include <QTextEdit>
#include <memory>

namespace omm
{

CodeEdit::CodeEdit(QWidget* parent)
{
  auto text_edit = std::make_unique<QTextEdit>(this);
  m_text_edit = text_edit.get();
  std::make_unique<QHBoxLayout>(this).release();
  layout()->addWidget(text_edit.release());
  layout()->setContentsMargins(0, 0, 0, 0);

  // see https://stackoverflow.com/a/54605709/4248972

  static constexpr int tab_width_char = 2;
  m_text_edit->setFont(QFont("Courier", 12));
  const auto font_metrics = m_text_edit->fontMetrics();

  static constexpr int big_number = 1000; // arbitrary big number.
  const QString test_string(" ");

  // compute the size of a char in double-precision
  const int single_char_width = font_metrics.width(test_string);
  const int many_char_width = font_metrics.width(test_string.repeated(big_number));
  const double single_char_width_double = many_char_width / double(big_number);

  // set the tab stop with double-precision
  m_text_edit->setTabStopDistance(tab_width_char * single_char_width_double);

  connect(m_text_edit, &QTextEdit::textChanged, [this]() {
    Q_EMIT this->code_changed(code());
  });
}

std::string CodeEdit::code() const
{
  return m_text_edit->toPlainText().toStdString();
}

void CodeEdit::clear()
{
  m_text_edit->clear();
}

void CodeEdit::set_code(const std::string& code)
{
  if (code != this->code()) {
    m_text_edit->setText(QString::fromStdString(code));
  }
}

void CodeEdit::set_placeholder_text(const std::string& ph)
{
  m_text_edit->setPlaceholderText(QString::fromStdString(ph));
}

}  // namespace omm
