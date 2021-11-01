#pragma once

#include "common.h"
#include "managers/manager.h"
#include <memory>
#include <stack>
#include <string>

namespace omm
{
class CodeEdit;
class ReferenceLineEdit;

class PythonConsole : public Manager
{
  Q_OBJECT
public:
  explicit PythonConsole(Scene& scene);
  ~PythonConsole() override;
  PythonConsole(PythonConsole&&) = delete;
  PythonConsole(const PythonConsole&) = delete;
  PythonConsole& operator=(PythonConsole&&) = delete;
  PythonConsole& operator=(const PythonConsole&) = delete;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PythonConsole");
  [[nodiscard]] QString type() const override;
  bool perform_action(const QString& name) override;

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  std::unique_ptr<QMenuBar> make_menu_bar();
  CodeEdit* m_commandline;
  CodeEdit* m_output;
  QVBoxLayout* m_layout;
  ReferenceLineEdit* m_associated_item_widget;
  void eval();
  bool accept(const void* associated_item) const;
  void clear();

  void get_previous_command();
  void get_next_command();
  void push_command(const QString& command);

  std::list<QString> m_command_stack;
  std::list<QString>::iterator m_command_stack_pointer = m_command_stack.end();

  static constexpr Qt::KeyboardModifiers caption_modifiers = Qt::ControlModifier;

private:
  void on_output(const void* associated_item, const QString& text, const Stream& stream);

private:
  // allocation on stack issues strange compiler warnings
  void* m_locals = nullptr;
};

}  // namespace omm
