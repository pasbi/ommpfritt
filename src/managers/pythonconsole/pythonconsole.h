#pragma once

#include <stack>
#include "managers/manager.h"
#include <memory>
#include "python/pythonengine.h"
#include <string>
#include "keybindings/commandinterface.h"

namespace omm
{

class CodeEdit;
class ReferenceLineEdit;

class PythonConsole : public Manager, public PythonIOObserver, public CommandInterface
{
public:
  explicit PythonConsole(Scene& scene);
  ~PythonConsole();

  void on_stdout(const void* associated_item, const std::string& text) override;
  void on_stderr(const void* associated_item, const std::string& text) override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PythonConsole");
  std::string type() const override;

protected:
  bool eventFilter(QObject* object, QEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

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

public:
  static std::map<std::string, QKeySequence> default_bindings();
  void call(const std::string& command) override;
  void  populate_menu(QMenu& menu) override;

};

}  // namespace omm
