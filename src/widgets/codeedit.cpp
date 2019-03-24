#include "widgets/codeedit.h"
#include <QHBoxLayout>
#include <QTextEdit>
#include <QKeyEvent>
#include <glog/logging.h>
#include <memory>
#include "common.h"
#include <QScrollBar>
#include "common.h"

namespace
{

std::string trim_trailing_newline(const std::string& s)
{
  if (s.size() == 0) { return s; }
  if (s.back() == '\n') { return s.substr(0, s.size() - 1); }
  return s;
}

}  // namespace

namespace omm
{

void TextEdit::keyPressEvent(QKeyEvent* event)
{
  const auto captioned_keys = std::set { Qt::Key_Return, Qt::Key_Down, Qt::Key_Up };
  if (::contains(captioned_keys, static_cast<Qt::Key>(event->key()))) {
    if (event->modifiers() == caption_modifiers) {
      event->setModifiers(Qt::NoModifier);
      QTextEdit::keyPressEvent(event);
      return;
    }
  } else {
    QTextEdit::keyPressEvent(event);
    return;
  }
  event->setAccepted(false);  // event is not yet finished.
}

CodeEdit::CodeEdit(QWidget* parent) : QWidget(parent)
{
  auto text_edit = std::make_unique<TextEdit>(this);
  m_text_edit = text_edit.get();
  std::make_unique<QHBoxLayout>(this).release();
  layout()->addWidget(text_edit.release());
  layout()->setContentsMargins(0, 0, 0, 0);

  // see https://stackoverflow.com/a/54605709/4248972

  static constexpr int tab_width_char = 2;
  m_text_edit->setFont(QFont("Courier", 12));
  const auto font_metrics = m_text_edit->fontMetrics();
  m_text_edit->setLineWrapMode(QTextEdit::NoWrap);

  static constexpr int big_number = 1000; // arbitrary big number.
  const QString test_string(" ");

  // compute the size of a char in double-precision
  const int many_char_width = font_metrics.width(test_string.repeated(big_number));
  const double single_char_width_double = many_char_width / double(big_number);

  // set the tab stop with double-precision
  m_text_edit->setTabStopDistance(tab_width_char * single_char_width_double);

  connect(m_text_edit, &QTextEdit::textChanged, [this]() {
    Q_EMIT this->code_changed(code());
    this->updateGeometry();
  });
}

std::string CodeEdit::code() const { return m_text_edit->toPlainText().toStdString(); }
void CodeEdit::clear() { m_text_edit->clear(); }
void CodeEdit::set_editable(bool editable) { m_text_edit->setReadOnly(!editable); }
void CodeEdit::set_caption_modifiers(const Qt::KeyboardModifiers &modifiers)
{
  m_text_edit->caption_modifiers = modifiers;
}

void CodeEdit::set_code(const std::string& code)
{
  if (code != this->code()) {
    m_text_edit->setText(QString::fromStdString(code));
    m_text_edit->moveCursor(QTextCursor::End);
  }
}

void CodeEdit::set_placeholder_text(const std::string& ph)
{
  m_text_edit->setPlaceholderText(QString::fromStdString(ph));
}

QSize CodeEdit::sizeHint() const
{
  const auto font_metrics = m_text_edit->fontMetrics();
  const auto lines = m_text_edit->toPlainText().split("\n");
  const auto widths = ::transform<int, std::vector>(lines, [&font_metrics](const QString& line) {
    return font_metrics.width(line);
  });
  const auto max_width = *std::max_element(widths.begin(), widths.end());

  const auto height = font_metrics.lineSpacing() * lines.size();
  return QSize(max_width, height);
}

void CodeEdit::scroll_to_bottom()
{
  auto* scroll_bar = m_text_edit->verticalScrollBar();
  scroll_bar->setValue(scroll_bar->maximum());
}

bool CodeEdit::is_at_bottom() const
{
  auto* scroll_bar = m_text_edit->verticalScrollBar();
  return scroll_bar->value() == scroll_bar->maximum();
}

void CodeEdit::put(const std::string& text, Stream stream)
{
  const bool was_at_bottom = is_at_bottom();
  switch (stream) {
  case Stream::stdout:
    m_text_edit->setTextColor(Qt::black);
    break;
  case Stream::stderr:
    m_text_edit->setTextColor(Qt::red);
    break;
  }
  m_text_edit->append(QString::fromStdString(trim_trailing_newline(text)));
  if (was_at_bottom) { scroll_to_bottom(); }
}

}  // namespace omm
