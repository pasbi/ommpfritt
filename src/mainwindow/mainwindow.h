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
  ~MainWindow();
  void restore_state();
  void save_state();
  std::unique_ptr<Manager> make_manager(const std::string& type);

  using creator_map = std::map<std::string, std::unique_ptr<Manager>(*)(Scene&)>;
  static const creator_map manager_creators;

protected:
  void closeEvent(QCloseEvent *event);

private:
  Application& m_app;
};

}  // namespace omm
