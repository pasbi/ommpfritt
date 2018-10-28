#pragma once

#include "scene/project.h"
#include <QObject>

class QApplication;

namespace omm
{

class MainWindow;

class Application : public QObject
{
  Q_OBJECT
public:
  Application(QApplication& app);
  bool save();
  bool save_as();
  bool save(const std::string& filename);
  bool can_close();
  bool load();
  void reset();
  void quit();
  void update_undo_redo_enabled();
  void new_project();
  void set_main_window(MainWindow& main_window);

  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    m_project.submit(std::make_unique<CommandT>(project(), std::forward<Args>(args)...));
  }

  Project& project();
  const Project& project() const;

private:
  QApplication& m_app;
  MainWindow* m_main_window;
  Project m_project;

};

}  // namespace omm
