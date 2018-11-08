#pragma once

#include <QObject>
#include "scene/scene.h"

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
  void set_main_window(MainWindow& main_window);
  void new_scene();


  Scene& scene();
  const Scene& scene() const;

private:
  QApplication& m_app;
  MainWindow* m_main_window;
  Scene m_scene;

};

}  // namespace omm
