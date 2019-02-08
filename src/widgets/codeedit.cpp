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
  m_text_edit->setFont(QFont("Courier", 12));
  m_text_edit->setTabStopDistance(fontMetrics().width("em"));
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
