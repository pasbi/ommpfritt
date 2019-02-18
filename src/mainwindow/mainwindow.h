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

  static std::map<std::string, std::list<std::string>> main_menu_entries();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;
  void add_menu(const std::string& title, const std::list<std::string>& actions);

};

}  // namespace omm
