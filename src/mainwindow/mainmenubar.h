#pragma once

#include <QMenuBar>

namespace omm
{

class MainWindow;
class Application;

class MainMenuBar : public QMenuBar
{
public:
  explicit MainMenuBar(Application& app, MainWindow& main_window);

private:
  void make_file_menu();
  void make_create_menu();
  void make_window_menu();
  void make_edit_menu();
  Application& m_app;
  MainWindow& m_main_window;
};

}  // namespace omm
