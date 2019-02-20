#pragma once

#include <QObject>
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "keybindings/keybindings.h"
#include "keybindings/commandinterface.h"
#include <Qt>

class QApplication;

namespace omm
{

class MainWindow;

class Application : public QObject, public CommandInterface
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

  void call(const std::string& command) override;
  static std::map<std::string, QKeySequence> default_bindings();
  static constexpr auto TYPE = QT_TR_NOOP("Application");
  std::string type() const override;

  PythonEngine python_engine;
  Scene scene;
  MainWindow* main_window() const;

private:
  QApplication& m_app;
  static Application* m_instance;
  MainWindow* m_main_window;

public:
  KeyBindings key_bindings;
};

}  // namespace omm
