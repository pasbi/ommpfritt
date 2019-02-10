#pragma once

#include <stack>
#include "managers/manager.h"
#include <memory>
#include "python/pythonengine.h"
#include <string>

namespace omm
{

class CodeEdit;
class ReferenceLineEdit;

class PythonConsole : public Manager, public PythonIOObserver
{
  DECLARE_MANAGER_TYPE(PythonConsole)

public:
  explicit PythonConsole(Scene& scene);
  ~PythonConsole();

  void on_stdout(const void* associated_item, const std::string& text) override;
  void on_stderr(const void* associated_item, const std::string& text) override;

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
  void push_command(const std::string& command);

  std::list<std::string> m_command_stack;
  std::list<std::string>::iterator m_command_stack_pointer = m_command_stack.end();

  static constexpr Qt::KeyboardModifiers caption_modifiers = Qt::ControlModifier;
};

}  // namespace omm
