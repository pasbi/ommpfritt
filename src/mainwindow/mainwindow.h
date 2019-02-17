#pragma once

#include <memory>
#include <map>
#include <QMainWindow>
#include "keybindings/keybindings.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class Application;
class Scene;
class Manager;

class MainWindow : public QMainWindow, CommandInterface
{
  Q_OBJECT
public:
  explicit MainWindow(Application& app);
  void restore_state();
  void save_state();
  void call(const std::string& command) override;
  static const std::map<std::string, QKeySequence> DEFAULT_BINDINGS;
  static constexpr auto TYPE = "MainWindow";
  std::string type() const override;
  void keyPressEvent(QKeyEvent* e) override;

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;
public:
  KeyBindings key_bindings;

};

}  // namespace omm
