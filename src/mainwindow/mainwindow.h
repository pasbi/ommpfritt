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
  ~MainWindow();
  void restore_state();
  void save_state();
  void call(const std::string& command) override;
  std::string type() const override;

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;
public:
  KeyBindings key_bindings;

};

}  // namespace omm
