#pragma once

#include <QObject>
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "keybindings/keybindings.h"
#include "keybindings/commandinterface.h"
#include <Qt>
#include "mainwindow/iconprovider.h"

class QApplication;

namespace omm
{

class MainWindow;
class Manager;

class Application : public QObject, public CommandInterface
{
  Q_OBJECT
public:
  Application(QApplication& app);
  bool save();
  bool save_as();
  bool save(const std::string& filename);
  bool can_close();
  void load();
  void reset();
  void quit();
  void update_undo_redo_enabled();
  void set_main_window(MainWindow& main_window);
  QKeySequence default_key_sequence(const std::string& name) const;
  static Application& instance();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Application");
  std::string type() const override;

  PythonEngine python_engine;
  Scene scene;
  MessageBox& message_box();
  MainWindow* main_window() const;
  const IconProvider icon_provider;

  enum class InsertionMode { Default, AsParent, AsChild };
  Object &insert_object(const std::string& key, InsertionMode mode);

private:
  QApplication& m_app;
  static Application* m_instance;
  MainWindow* m_main_window;

public:
  KeyBindings key_bindings;

public:
  bool dispatch_key(int key);
  void register_manager(Manager& manager);
  void unregister_manager(Manager& manager);
  bool perform_action(const std::string& name) override;
private:
  QTimer m_reset_keysequence_timer;
  std::vector<int> m_pending_key_sequence;
  std::set<Manager*> m_managers;

};

}  // namespace omm
