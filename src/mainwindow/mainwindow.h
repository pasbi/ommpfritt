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

  static std::vector<std::string> object_menu_entries();
  static std::vector<std::string> path_menu_entries();
  static std::vector<std::string> main_menu_entries();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;

};

}  // namespace omm
