#pragma once

#include <QWidget>

class QTextEdit;

namespace omm
{

class CodeEdit : public QWidget
{
  Q_OBJECT
public:
  explicit CodeEdit(QWidget* parent);
  std::string code() const;
  void clear();
  void set_code(const std::string& code);
  void set_placeholder_text(const std::string& ph);

Q_SIGNALS:
  void code_changed(const std::string& code);

private:
  QTextEdit* m_text_edit;
};

}  // namespace omm
