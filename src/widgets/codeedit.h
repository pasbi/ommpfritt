#pragma once

#include <QWidget>
#include <QTextEdit>

namespace omm
{

class TextEdit : public QTextEdit
{
public:
  using QTextEdit::QTextEdit;
  Qt::KeyboardModifiers caption_modifiers = Qt::NoModifier;
protected:
  void keyPressEvent(QKeyEvent* event) override;
};


class CodeEdit : public QWidget
{
  Q_OBJECT
public:
  enum class Stream { stdout, stderr };
  explicit CodeEdit(QWidget* parent = nullptr);
  std::string code() const;
  void clear();
  void set_code(const std::string& code);
  void set_placeholder_text(const std::string& ph);
  QSize sizeHint() const override;
  void set_editable(bool editable);
  void put(const std::string& text, Stream stream);
  void scroll_to_bottom();
  bool is_at_bottom() const;
  void set_caption_modifiers(const Qt::KeyboardModifiers& modifier);

Q_SIGNALS:
  void code_changed(const std::string& code);

private:
  TextEdit* m_text_edit;
};

}  // namespace omm
