#pragma once

#include <QWidget>
#include <QTextEdit>
#include "common.h"

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
  explicit CodeEdit(QWidget* parent = nullptr);
  QString code() const;
  void clear();
  void set_code(const QString& code);
  void set_placeholder_text(const QString& ph);
  QSize sizeHint() const override;
  void set_editable(bool editable);
  void put(const QString& text, Stream stream);
  void scroll_to_bottom();
  bool is_at_bottom() const;
  void set_caption_modifiers(const Qt::KeyboardModifiers& modifier);

Q_SIGNALS:
  void code_changed(const QString& code);

private:
  TextEdit* m_text_edit;
};

}  // namespace omm
