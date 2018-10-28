#pragma once

#include <QMenuBar>

namespace omm
{

class Application;

class MainMenuBar : public QMenuBar
{
public:
  explicit MainMenuBar(Application& app);

private:
  void make_file_menu();
  void make_create_menu();
  Application& m_app;
};

}  // namespace omm
