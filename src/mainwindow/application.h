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
  QKeySequence default_key_sequence(const std::string& name) const;
  static Application& instance();

  static std::vector<CommandInterface::ActionInfo<Application>> action_infos();
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Application");
  std::string type() const override;

  PythonEngine python_engine;
  Scene scene;
  MainWindow* main_window() const;

private:
  QApplication& m_app;
  static Application* m_instance;
  MainWindow* m_main_window;

  std::map<std::string, QKeySequence> m_default_key_sequences;

public:
  KeyBindings key_bindings;
};

}  // namespace omm
