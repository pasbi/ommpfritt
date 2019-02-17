#pragma once

#include <QObject>
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "keybindings/keybindings.h"

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
  static Application& instance();

  PythonEngine python_engine;
  Scene scene;

private:
  QApplication& m_app;
  static Application* m_instance;
  MainWindow* m_main_window;

public:
  KeyBindings key_bindings;
};

}  // namespace omm
