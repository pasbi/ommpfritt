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
  Application& m_app;
  MainWindow& m_main_window;

  template<typename ManagerT> void show_manager();

};

}  // namespace omm
