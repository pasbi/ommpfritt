#pragma once

#include <memory>
#include <map>
#include <QMainWindow>
#include "keybindings/keybindings.h"

namespace omm
{

class Application;
class Scene;
class Manager;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Application& app);
  void restore_state();
  void save_state();
  void keyPressEvent(QKeyEvent* e) override;

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;
  void add_menu(const std::string& title, const std::vector<std::string>& actions);
};

}  // namespace omm
