#pragma once

#include <memory>
#include <map>
#include <QMainWindow>

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

protected:
  void closeEvent(QCloseEvent *event);

private:
  Application& m_app;
};

}  // namespace omm
